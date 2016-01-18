VSRagel
===============

## What is VSRagel
VSRagel is a Visual Studio extension for the [Ragel State Machine Compiler](http://www.colm.net/open-source/ragel/) that adds syntax highlighting and code generation capabilities.

This is an early alpha version.

## Notes

The extensions creates C# code files.

The Ragel source code has been modified to support Unicode symbols in the input files. The predefined character classes have not been modified yet. Unfortunately the Ruby code generator has been broken as a result of those changes but since it is not used in this extension it is not a critical issue.
