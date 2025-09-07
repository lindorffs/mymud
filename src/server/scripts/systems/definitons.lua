Systems = {
	Cygnus_Prime = {
		systemName = "Cygnus Prime",
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
				location = { x = 4, y = 7 },
			},
			Jumpgate_Delta_Pyre = {
				siteName = "Jumpgate - Delta Pyre",
				description = "",
				rewards = {},
				connections = { {target_system="Delta_Pyre", target_proximity="Jumpgate_Cygnus_Prime", distance = 12} },
				location = { x = 8, y = 3 },
			}
		},
		location = { x = 6, y = 6 },
	},
	Delta_Pyre = {
		systemName = "Delta Pyre",
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
				location = { x = 1, y = 1},
			},
			Jumpgate_Cygnus_Prime = {
				siteName = "Jumpgate - Cygnus Prime",
				description = "",
				rewards = {},
				connections = { {target_system="Cygnus_Prime", target_proximity="Jumpgate_Delta_Pyre", distance = 12} },
				location = { x = 5, y = 8},
			},
			Jumpgate_Andromeda_Nexus = {
				siteName = "Jumpgate - Andromeda Nexus",
				description = "",
				rewards = {},
				connections = { {target_system="Andromeda_Nexus", target_proximity="Jumpgate_Delta_Pyre", distance = 12} },
				location = { x = 65, y = 72},
			},
		},
		location = { x = -16, y = -16 },
	},
	Andromeda_Nexus = {
		systemName = "Andromeda Nexus",
		description = "A bustling trade hub in the Andromeda galaxy.",
		players = {},
		Sites = {
			Grand_Exchange = {
				siteName = "Grand Exchange",
				description = "The largest marketplace in the sector.",
				rewards = {
					trade = 2
				},
				connections = {},
				location = { x = 30, y = 20 },
			},
			Ancient_Observatory = {
				siteName = "Ancient Observatory",
				description = "A relic from a bygone era, now used for stellar research.",
				rewards = {
					explore = 1
				},
				connections = {},
				location = { x = -70, y = 90 },
			},
			Jumpgate_Epsilon_Draconis = {
				siteName = "Jumpgate - Epsilon Draconis",
				description = "",
				rewards = {},
				connections = { {target_system="Epsilon_Draconis", target_proximity="Jumpgate_Andromeda_Nexus", distance = 15} },
				location = { x = 10, y = -15 },
			},
		},
		location = { x = 10, y = 15 },
	},
	Epsilon_Draconis = {
		systemName = "Epsilon Draconis",
		description = "Known for its harsh, volcanic planets and valuable mineral deposits.",
		players = {},
		Sites = {
			Volcanic_Mines = {
				siteName = "Volcanic Mines",
				description = "Rich in rare minerals but dangerous to extract.",
				rewards = {
					mining = 2
				},
				connections = {},
				location = { x = 60, y = -10 },
			},
			Refugee_Outpost = {
				siteName = "Refugee Outpost",
				description = "A small settlement for those escaping conflict in other systems.",
				rewards = {
					diplomacy = 1
				},
				connections = {},
				location = { x = 20, y = 40 },
			},
			Jumpgate_Andromeda_Nexus = {
				siteName = "Jumpgate - Andromeda Nexus",
				description = "",
				rewards = {},
				connections = { {target_system="Andromeda_Nexus", target_proximity="Jumpgate_Epsilon_Draconis", distance = 15} },
				location = { x = -90, y = 70 },
			},
		},
		location = { x = 20, y = 5 },
	},
	Orion_Nebula = {
		systemName = "Orion Nebula",
		description = "A system shrouded in a beautiful but treacherous nebula.",
		players = {},
		Sites = {
			Gas_Giant_Refinery = {
				siteName = "Gas Giant Refinery",
				description = "Processing station for gases extracted from the nebula.",
				rewards = {
					resource_gathering = 2
				},
				connections = {},
				location = { x = 55, y = -50 },
			},
			Pirate_Den = {
				siteName = "Pirate Den",
				description = "A hidden base for space pirates.",
				rewards = {
					combat = 2,
					illegal_trade = 1
				},
				connections = {},
				location = { x = -84, y = 20 },
			},
			Jumpgate_Vega_Prime = {
				siteName = "Jumpgate - Vega Prime",
				description = "",
				rewards = {},
				connections = { {target_system="Vega_Prime", target_proximity="Jumpgate_Orion_Nebula", distance = 18} },
				location = { x = 30, y = 80 },
			},
		},
		location = { x = -10, y = 20 },
	},
	Vega_Prime = {
		systemName = "Vega Prime",
		description = "A vibrant system with advanced technology and a strong research presence.",
		players = {},
		Sites = {
			Unified_Research_Center = {
				siteName = "Unified Research Center",
				description = "Leading institution for scientific breakthroughs.",
				rewards = {
					research = 3
				},
				connections = {},
				location = { x = 10, y = 80 },
			},
			Luxury_Resort = {
				siteName = "Luxury Resort",
				description = "A popular destination for intergalactic travelers.",
				rewards = {
					influence = 1
				},
				connections = {},
				location = { x = -44, y = 31 },
			},
			Jumpgate_Orion_Nebula = {
				siteName = "Jumpgate - Orion Nebula",
				description = "",
				rewards = {},
				connections = { {target_system="Orion_Nebula", target_proximity="Jumpgate_Vega_Prime", distance = 18} },
				location = { x = 70, y = -60 },
			},
		},
		location = { x = -25, y = 10 },
	},
	Zenith_Cluster = {
		systemName = "Zenith Cluster",
		description = "A newly discovered cluster of stars, ripe for exploration.",
		players = {},
		Sites = {
			Uncharted_Planet_Alpha = {
				siteName = "Uncharted Planet Alpha",
				description = "A pristine, unexplored world.",
				rewards = {
					explore = 2
				},
				connections = {},
				location = { x = 25, y = -63 },
			},
			Ancient_Alien_Ruin = {
				siteName = "Ancient Alien Ruin",
				description = "Remnants of an advanced civilization.",
				rewards = {
					archaeology = 2
				},
				connections = {},
				location = { x = 90, y = 40 },
			},
		},
		location = { x = 30, y = 25 },
	},
}