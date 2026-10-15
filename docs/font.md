---
title: Hershey Font Files
---

# Hershey Font Files

GrADS supports Hershey fonts numbered 0 to 9. Fonts 0 to 5 are provided with the GrADS distribution. The files are named `font0.dat`, `font1.dat`, etc. If you create a `font6.dat` file (or 7 through 9) and put it in the same place as the other font files, you can use that font immediately, by using the <a href="gradcomdsetfont.html">`set font`</a> command, or via <a href="fontcontrol.html">font escape sequences</a>.

The font files are in an ASCII format. Each file is 95 records in length. Each record represents a character in the font, and the records are ordered in the order of ASCII code values, starting with the blank (code value 32 decimal). So the last record represents the tilde, the 17th record the zero, etc. So when you are using the font the file represents, and enter a zero character, the character in the 17th record is plotted, whatever it may be.

Each record starts with a 3 character number. This is the number of code pairs in the record. This number is followed by the indicated number of code pairs. Each code pair is two characters. Each character represents a value, which is determined as an offset from the Capital R (decimal 82). So, if the character is Capital L, it represents a value of -6. If the character is a lower case k, it represents a value of +25.

The first code pair represents the horizontal extent of the character. So the first character of the code pair would be the minimum X, and the 2nd character the maximum X. If I remember correctly, this extent should include white space. This is followed by code pairs that represent drawing positions in X and Y, where the first character is X, and the 2nd Y. A "pen up" is indicated by the code pair " R" (blank, followed by capital R).

You can look at the existing font files for examples. If you look at `font0.dat`, the first record represents the blank. It thus has one code pair, which just represents the width of the blank in the font, thus allocating white space when a blank is encountered. If you look at record `57` (which represents Cap X), you see: `6H\KFY[ RYFK[` Decoding this, you see there are 6 code pairs. The first is the width extent, `H\`, which is -10 to 10. The next two pairs, `KFY[`, are points -7,-12 and 7,9. So a line would be drawn between those two points (appropriate scaled). The next code pair indicates pen up, followed by `YFK[`, which are 7,-12 and -7,9.

You can see the horizontal extent does not match too well with the actual character. I am not quite sure why this is, nor why the character is not centered. This is the way the fonts came, so I assume there are some font design issues involved.

If you want to design your own font, you will need to review the code GrADS uses to actually plot these fonts, which is `gxchpl.c`. I determined scale factors and centering issues by trial and error, and these values are contained in the code.
