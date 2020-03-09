BackgroundColor = 0x29cdb5fa
Font = "Fonts/Teko-Light.ttf"
BoxAnimationSpeed = 3
Nickname = { fontSize = 34, colorForMine = 0xffa500ff, colorForOthers = 0xffffffff } -- NOTE: Font is the same as that of GuideTextLabel.
Box = { x = 0, y = 100, 
		width = 800, height = 400,
		color = 0x0f0f0fff,
		outlineThickness = 5, outlineColor = 0x0000007f }
MovingPoints = { {50, 450},
				{50, 110},
				{650, 110},
				{650, 450} }
GuideTextLabel = { x = 420, y = 400,
					color = 0xffffff7f, fontSize = 34 }
GuideTexts = { "Double click to create a room.",
				"Right click to join a room." }
SubWindow = { shade = 0x0000007f,
			  x = 100, y = 250, width = 600, height = 100,
			  color = 0x29cdb5fa,
			  font = "Fonts/Teko-Regular.ttf",
			  titleRelativeX = 20, titleRelativeY = 10, titleFontSize = 32, titleFontColor = 0xffffffff,
			  inputTextFieldX = 20, inputTextFieldY = 50, inputTextFieldFontSize = 30, inputTextFieldFontColor = 0x000000ff }
Sound = { onSelection = { path = "Sounds/selection.wav" } }