--BackgroundColor = 0x8ae5ffff --SKY
BackgroundColor = 0x29cdb5fa --CYAN
Font = "Fonts/AGENCYR.TTF"
PlayerPanel = { x = 100, y = 0, cellSize = 30,
				color = 0x1f1f1fff,
				outlineThickness = 11, outlineColor = 0x3f3f3f7f,
				nicknameFontSize = 30, nicknameFontColor = 0xffa500ff } -- Orange
CountdownSprite = { path = "Images/Countdown.png", clipWidth = 256, clipHeight = 256 }
VfxCombo = { path = "Vfxs/Combo.png", clipWidth = 256, clipHeight = 256 }
NextTetriminoPanel = { x = 600, y = 110, cellSize = 30,
					color = 0x1f1f1fff,
					outlineThickness = 5, outlineColor = 0x3f3f3f7f,
					angularVelocity = 1.5, arcLength = 90, rotationIntervalMs = 2000, scaleFactor = 0.7 }
OtherPlayerSlot = { x = 600, y = 400, cellSize = 8,
					margin = 10,
					color = 0x1f1f1f7f, color_on = 0x1f1f1fff,
					outlineThickness = 2, outlineColor = 0x3f3f3f7f,
					outlineThickness_on = 5, outlineColor_on = 0x3f3f3f7f,
					nicknameFontSize = 20, nicknameFontColor = 0xffffffff }
StartingGuide = { x = 600, y = 100, fontSize = 40, fontColor = 0xffffffff }