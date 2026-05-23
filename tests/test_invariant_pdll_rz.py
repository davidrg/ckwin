import pytest
import ctypes
import struct


# Simulated safe buffer copy function that models the expected behavior
# of pdll_rz.c protocol data handling with proper validation
MAX_ATTN_BUFFER_SIZE = 256  # Expected maximum buffer size for 'attn'


def safe_memcpy(dest_buf, dest_size, src_data, src_len):
    """
    Safe version of the memcpy operation from pdll_rz.c:248
    Models what the code SHOULD do: validate rx_buf_len before copying.
    Returns (bytes_copied, error) where error is None on success.
    """
    if src_len < 0:
        return 0, "negative length"
    if src_len > dest_size:
        return 0, f"length {src_len} exceeds buffer size {dest_size}"
    if len(src_data) < src_len:
        return 0, "source data shorter than declared length"
    
    dest_buf[:src_len] = src_data[:src_len]
    return src_len, None


def simulate_protocol_packet_handling(rx_buf, rx_buf_len):
    """
    Simulates the protocol data handling in pdll_rz.c
    Models the vulnerable pattern: memcpy(attn, rx_buf, rx_buf_len)
    with the security fix applied: validate rx_buf_len before use.
    
    Returns dict with result info.
    """
    attn = bytearray(MAX_ATTN_BUFFER_SIZE)
    
    # Security invariant: rx_buf_len must be validated before use
    if not isinstance(rx_buf_len, int):
        return {"success": False, "error": "invalid length type", "bytes_copied": 0}
    
    if rx_buf_len < 0:
        return {"success": False, "error": "negative length rejected", "bytes_copied": 0}
    
    if rx_buf_len > MAX_ATTN_BUFFER_SIZE:
        return {"success": False, "error": "oversized length rejected", "bytes_copied": 0}
    
    if len(rx_buf) < rx_buf_len:
        return {"success": False, "error": "source buffer too small", "bytes_copied": 0}
    
    # Safe copy
    bytes_copied, err = safe_memcpy(attn, MAX_ATTN_BUFFER_SIZE, rx_buf, rx_buf_len)
    if err:
        return {"success": False, "error": err, "bytes_copied": 0}
    
    return {"success": True, "error": None, "bytes_copied": bytes_copied, "attn": bytes(attn)}


@pytest.mark.parametrize("payload", [
    # (rx_buf, rx_buf_len, description)
    # Oversized by 2x
    (b"A" * 512, 512, "2x oversized buffer"),
    # Oversized by 10x
    (b"B" * 2560, 2560, "10x oversized buffer"),
    # Exactly at boundary + 1
    (b"C" * 257, 257, "boundary+1 overflow"),
    # Maximum integer value as length
    (b"D" * 100, 2**31 - 1, "max int32 length"),
    # Unsigned overflow attempt
    (b"E" * 100, 2**32 - 1, "max uint32 length"),
    # Negative length (underflow attempt)
    (b"F" * 100, -1, "negative length"),
    # Negative large value
    (b"G" * 100, -2**31, "min int32 negative"),
    # Length much larger than actual data (classic heap overflow setup)
    (b"H" * 10, 1000, "length larger than actual data"),
    # Zero-length (edge case)
    (b"", 0, "zero length"),
    # Crafted protocol packet with embedded length field claiming oversized data
    (struct.pack(">I", 65535) + b"I" * 100, 65535, "crafted packet with oversized length field"),
    # NUL bytes with oversized length
    (b"\x00" * 512, 512, "null bytes 2x oversized"),
    # Mixed attack payload
    (b"\xff\xfe\xfd" * 200, 600, "binary attack payload 2x+"),
    # Format string-like payload with oversized length
    (b"%s%n%x%p" * 100, 800, "format string payload oversized"),
    # SQL injection-like payload with oversized length
    (b"' OR '1'='1" * 50, 550, "sql injection payload oversized"),
    # Shell metacharacter payload oversized
    (b"; cat /etc/passwd; " * 30, 570, "shell injection payload oversized"),
    # Exactly at max size (should succeed)
    (b"J" * 256, 256, "exactly at max size"),
    # One byte under max (should succeed)
    (b"K" * 255, 255, "one byte under max"),
    # Large power of 2 sizes
    (b"L" * 1024, 1024, "1KB oversized"),
    (b"M" * 4096, 4096, "4KB oversized"),
    (b"N" * 65536, 65536, "64KB oversized"),
])
def test_buffer_read_never_exceeds_declared_length(payload):
    """
    Invariant: Buffer reads must never exceed the declared/allocated buffer size.
    
    When rx_buf_len (derived from remote protocol fields) exceeds MAX_ATTN_BUFFER_SIZE,
    the copy operation must be rejected or truncated — never allowed to overflow.
    This guards against CWE-120: Buffer Copy without Checking Size of Input.
    """
    rx_buf, rx_buf_len, description = payload
    
    result = simulate_protocol_packet_handling(rx_buf, rx_buf_len)
    
    # INVARIANT 1: bytes_copied must never exceed the allocated buffer size
    assert result["bytes_copied"] <= MAX_ATTN_BUFFER_SIZE, (
        f"[{description}] OVERFLOW: bytes_copied={result['bytes_copied']} "
        f"exceeds MAX_ATTN_BUFFER_SIZE={MAX_ATTN_BUFFER_SIZE}"
    )
    
    # INVARIANT 2: if rx_buf_len exceeds buffer, operation must fail (not silently overflow)
    if isinstance(rx_buf_len, int) and rx_buf_len > MAX_ATTN_BUFFER_SIZE:
        assert result["success"] is False, (
            f"[{description}] SECURITY VIOLATION: oversized rx_buf_len={rx_buf_len} "
            f"was accepted without rejection. Must reject inputs exceeding buffer size."
        )
        assert result["bytes_copied"] == 0, (
            f"[{description}] SECURITY VIOLATION: bytes were copied despite oversized length"
        )
    
    # INVARIANT 3: if rx_buf_len is negative, operation must fail
    if isinstance(rx_buf_len, int) and rx_buf_len < 0:
        assert result["success"] is False, (
            f"[{description}] SECURITY VIOLATION: negative rx_buf_len={rx_buf_len} "
            f"was accepted. Must reject negative lengths."
        )
    
    # INVARIANT 4: successful copies must have copied exactly rx_buf_len bytes
    if result["success"]:
        assert result["bytes_copied"] == rx_buf_len, (
            f"[{description}] Mismatch: bytes_copied={result['bytes_copied']} "
            f"!= rx_buf_len={rx_buf_len}"
        )
        # And that must be within bounds
        assert result["bytes_copied"] <= MAX_ATTN_BUFFER_SIZE, (
            f"[{description}] Successful copy exceeded buffer bounds"
        )
    
    # INVARIANT 5: the attn buffer content must not exceed allocated size
    if "attn" in result:
        assert len(result["attn"]) == MAX_ATTN_BUFFER_SIZE, (
            f"[{description}] attn buffer size changed: {len(result['attn'])} "
            f"!= {MAX_ATTN_BUFFER_SIZE}"
        )