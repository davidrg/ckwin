/* Create the version string for Kermit with the last compi-	*/
/* lation time. Compile this resource definition with Rez.		*/
/* Created: 10/12/87 Matthias Aebi								*/
/* Modifications:												*/

type 'KR09' {
	byte = 40;		/* (40) the string length */
	array CompileTime {
		string;
		byte;		/* month first digit */
		byte;		/* month second digit */
		byte = '/';
		byte;		/* day first digit */
		byte;		/* day second digit */
		byte = '/';
		byte;		/* year first digit */
		byte;		/* year second digit */
		byte = ' ';
		byte;		/* hour first digit */
		byte;		/* hour second digit */
		byte = ':';
		byte;		/* minute first digit */
		byte;		/* minute second digit */
	};
};

resource 'KR09' (0) {
	{	"Kermit Version 0.98(63) Ñ ",
		($$Month/10)+48,
		($$Month%10)+48,
		($$Day/10)+48,
		($$Day%10)+48,
		($$Year%100/10)+48,
		($$Year%10)+48,
		($$Hour/10)+48,
		($$Hour%10)+48,
		($$Minute/10)+48,
		($$Minute%10)+48
	}
};
