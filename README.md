# vdent: Verilog Indenter

Simple indent program for Verilog source code. Trims end of line white space and indents lines based on nested depth of code blocks.

## Usage

```
vdent [-h] [-s num] < input.v [ > output.v ]
```

The vdent program will produced consistently indented verilog code. However, it is not a code formatter or beautifier in the sense that it leaves vertical white space alone, does not pad operators or brackets etc and trusts that, sometimes YOU KNOW BEST. Examples of when vdent leaves well enough alone is the leading white space in front of line comments and line breaks within 'assign' statements.

For the above reasons vdent is often used as a first pass filter that does the heavy lifting of indenting code but leaves it in a state that will need a small amount of hand formatting to get it "just right".

## Indent as spaces

The use of spaces or tab characters for indent is a subject of much passions for many developers, I'm not one of them. The current default of spaces as the indent was arbitrarily made to make coding simpler.

Tools to convert indent from/to spaces and tabs already exist in the form of "expand"/"unexpand". These commands are part of the Coreutils and should as such be available on any UNIX based system.

```
vdent -s8 < original.v | unexpand > indented_with_tabs.v
```


