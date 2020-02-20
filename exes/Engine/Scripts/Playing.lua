BackgroundColor = 0x29cdb5fa --CYAN
BlackOutColor = 0x808080ff --GRAY
PlayerPanel = { x = 130, y = 0, cellSize = 30,
				color = 0x1f1f1fff,
				outlineThickness = 11, outlineColor = 0x3f3f3f7f,
				cellOutlineColor = 0x0000007f }
VfxCombo = { path = "Vfxs/Combo.png", clipWidth = 256, clipHeight = 256 }
NextTetriminoPanel = { x = 620, y = 180, cellSize = 30,
					   color = 0x1f1f1fff,
					   outlineThickness = 5, outlineColor = 0x0000007f,
					   cellOutlineColor = 0x0000007f }
Sound = { tetriminoLocked = { path = "Sounds/tetriminoLocked.wav" },
		  lineCleared = { path = "Sounds/lineCleared.wav" },
		  levelCleared = { path = "Sounds/levelCleared.wav" } }
Missions = { { 7, 0.75 } },
			{ 13, 0.7 } }
Score = { path = "Images/Score.png", clipWidth = 128, clipHeight = 128,
		  x = 530, y = 330, gap = 60, animationSpeed = 1.0 }