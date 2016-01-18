%%{
	machine sample ## missing semicolon

	main := |*
		'{' { yield return Emit(TokenType.LCurly); };
		'}' { yield return Emit(TokenType.RCurly); };
		space;
		*|;
}%%
