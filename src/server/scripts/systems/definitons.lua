Systems = {
	Cygnus_Prime = {
		description = "Cygnus Prime",
		players = {},
		Sites = {
			State_Military_Academy = {
				siteName = "State Military Academy",
				description = "",
				rewards = {
					combat = 1
				},
				connections = {},
			},
			Jumpgate_Delta_Pyre = {
				siteName = "Jumpgate - Delta Pyre",
				description = "",
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
				siteName = "State Science Academy",
				description = "",
				rewards = {
					explore = 1
				},
				connections = {},
			},
			Jumpgate_Cygnus_Prime = {
				siteName = "Jumpgate - Cygnus Prime",
				description = "",
				rewards = {},
				connections = { {target_system="Cygnus_Prime", target_proximity="Jumpgate_Delta_Pyre", distance = 12} },
			},
		}
	},
}