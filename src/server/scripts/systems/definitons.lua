Systems = {
	Cygnus_Prime = {
		description = "Cygnus Prime",
		players = {},
		Sites = {
			State_Military_Academy = {
				description = "State Military Acadaemy",
				rewards = {
					combat = 1
				},
				connections = {},
			},
			Jumpgate_Delta_Pyre = {
				description = "Jumpgate - Delta Pyre",
				rewards = {},
				connections = { {target_system="Delta_Pyre", target_proximity="Jumpgate_Cygnus_Prime", distance = 12} },
			}
		}
	},
	Delta_Pyre = {
		description = "Delta Pyre",
		players = {},
		Sites = {
			State_Science_Academy = {
				description = "State Science Academy",
				rewards = {
					explore = 1
				},
				connections = {},
			},
			Jumpgate_Cygnus_Prime = {
				description = "Jumpgate - Cygnus Prime",
				rewards = {},
				connections = { {target_system="Cygnus_Prime", target_proximity="Jumpgate_Delta_Pyre", distance = 12} },
			},
		}
	},
}