local function createSystem(name, description, Sites, location)
	return {
		systemName = name,
		description = description,
		location = location,
		players = {},
		Sites = Sites,
	}
end

local function createSite(name, description, rewards, connections, location, objects)
	return {
		siteName = name,
		description = description,
		rewards = rewards,
		connections = connections,
		location = location,
		objects = objects,
		entities = {},
	}
end

local function createObject(name, location)
	return {
		name = name,
		location = location,
	}
end

local function createConnection(target_system, target_proximity)
	return {
		target_system = target_system,
		target_proximity = target_proximity,
	}
end


Systems = {
	Cygnus_Prime = createSystem("Cygnus Prime", "Home worlds of the Alterran People.", {
		Alterra = createSite("Alterra", "Temperate Planet - Home of the Alterrans", {trade=1}, {}, {x=32, y=12}, {createObject("Alterra", {x=10, y = 10})}),
		State_Military_Academy = createSite("State Military Academy", "Station - Training grounds for the Alterran Military.", {combat=1}, {}, {x=10, y = 13}, {}),
		State_Science_Academy = createSite("State Science Academy", "Station - The premiere science academy of the Alterran people.", {explore=1}, {}, {x=13, y = 10}, {}),
		Jumpgate_Magnar_Solis = createSite("Jumpgate - Magnar Solis", "", {}, {createConnection("Magnar_Solis", "Jumpgate_Cygnus_Prime")}, {x=32, y=0}, {Jumpgate=createObject("Jumpgate", {x=-12,y=-12})})
	}, {x = 32, y = 60}),
	Magnar_Solis = createSystem("Magnar Solis", "Alterran Border System.", {
		Magnar_I = createSite("Magnar I", "Molten Planet", {trade=1}, {}, {x=15,y=-12}, {createObject("Magnar I", {x=10,y=10})}),
		Asteroid_Belt_I = createSite("Asteroid Belt I", "", {mining=1}, {}, {x=-44,y=-23}, {asteroid_1=createObject("Asteroid", {x=32,y=32}),asteroid_2=createObject("Asteroid", {x=30,y=32}),asteroid_3=createObject("Asteroid", {x=34,y=32})}),
		Jumpgate_Cygnus_Prime = createSite("Jumpgate - Cygnus Prime", "", {}, {createConnection("Cygnus_Prime", "Jumpgate_Magnar_Solis")}, {x=-35, y=-48}, {Jumpgate=createObject("Jumpgate", {x=14,y=10})}),
		-- Closest system is Orion_Nebula
		Jumpgate_Orion_Nebula = createSite("Jumpgate - Orion Nebula", "", {}, {createConnection("Orion_Nebula", "Jumpgate_Magnar_Solis")}, {x=0,y=-80}, {Jumpgate=createObject("Jumpgate", {x=-5,y=-15})})
	}, {x = 35, y = 44}),
	Vega_Prime = createSystem("Vega Prime", "Home worlds of the Pytheran People.", {
		Pythera = createSite("Pythera", "Gaseous Planet - Home of the Pytherans", {}, {}, {x=16, -20}, {createObject("Pythera", {x=-10,y=-10})}),
		Militia_Training_Facility = createSite("Militia Training Facility", "Station - Training grounds for the Pytheran Militia.", {combat=1}, {}, {x=-13,y=-10}, {}),
		Science_Training_Facility = createSite("Science Training Faility", "Station - The premiere science academy of the Pytheran people.", {explore=1}, {}, {x=-10,y=-13}, {}),
		Jumpgate_Centauri_Six = createSite("Jumpgate - Centauri Six", "", {}, {createConnection("Centauri_Six", "Jumpgate_Vega_Prime")}, {x=15,-15}, {Jumpgate=createObject("Jumpgate", {x=-12,y=12})})
	}, { x = -26, y = -80}),
	Centauri_Six = createSystem("Centauri Six", "Pytheran Border System.", {
		Jumpgate_Vega_Prime = createSite("Jumpgate - Vega Prime", "", {}, {createConnection("Vega_Prime", "Jumpgate_Centauri_Six")}, {x=15,y=-15}, {Jumpgate=createObject("Jumpgate", {x=-12,y=12})}),
		Asteroid_Belt_I = createSite("Asteroid Belt I", "", {mining=1}, {}, {x=-44,y=-23}, {asteroid_1=createObject("Asteroid", {x=32,y=32}),asteroid_2=createObject("Asteroid", {x=30,y=32}),asteroid_3=createObject("Asteroid", {x=34,y=32})}),
		-- Closest system is Betelgeuse
		Jumpgate_Betelgeuse = createSite("Jumpgate - Betelgeuse", "", {}, {createConnection("Betelgeuse", "Jumpgate_Centauri_Six")}, {x=100,y=100}, {Jumpgate=createObject("Jumpgate", {x=10,y=10})})
	}, {x = -23, y = -64}),
	
	Athena_Prime = createSystem("Athena Prime", "Home worlds of the Cascadian People.", {
		Cascadia = createSite("Cascadia", "Ice Planet - Home of the Cascadians", {trade=1}, {}, {x=12,y=-55}, {createObject("Cascadia", {x=10,y=-10})}),
		Peoples_Defense_Coallition = createSite("Peoples Defense Coallition", "Station - Training grounds for the Cascadian Defense Coallition", {combat=1}, {}, {x=13,y=-10}, {}),
		Peoples_Science_Academy = createSite("Peoples Science Acadaemy", "Station - The premiere science academy of the Cascadian People.", {explore=1}, {}, {x=10,y=-13},{}),
		Jumpgate_Smybal_Ashes = createSite("Jumpgate - Symbal Ashes", "", {}, {createConnection("Symbal_Ashes", "Jumpgate_Athena_Prime")}, {x=100,y=100}, {Jumpgate=createObject("Jumpgate", {x=10,y=10})}),
	}, { x = -32, y=12}),
	Symbal_Ashes = createSystem("Symbal Aashes", "Cascadian Border System.", {
		Jumpgate_Athena_Prime = createSite("Jumpgate - Athena Prime", "", {}, {createConnection("Athena_Prime", "Jumpgate_Smybal_Ashes")}, {x=15,y=-15}, {Jumpgate=createObject("Jumpgate", {x=-12,y=12})}),
		Asteroid_Belt_I = createSite("Asteroid Belt I", "", {mining=1}, {}, {x=-44,y=-23}, {asteroid_1=createObject("Asteroid", {x=32,y=32}),asteroid_2=createObject("Asteroid", {x=30,y=32}),asteroid_3=createObject("Asteroid", {x=34,y=32})}),
		-- Closest system is Saiph
		Jumpgate_Saiph = createSite("Jumpgate - Saiph", "", {}, {createConnection("Saiph", "Jumpgate_Symbal_Ashes")}, {x=100,y=-100}, {Jumpgate=createObject("Jumpgate", {x=15,y=-10})})
	}, { x = -19, y = 12}),
-- =================================================================
-- Orion Constellation Sector (Final Version)
-- Centered at: 35, -35
-- =================================================================
-- The Shoulders
	Betelgeuse = createSystem("Betelgeuse", "A colossal red supergiant nearing the end of its life. The system is bathed in a dim, crimson light, and its outer planets are rich in heavy elements forged in the star's heart. A key resource hub for rare materials.", {
		Jumpgate_Bellatrix = createSite("Jumpgate - Bellatrix", "", {}, {createConnection("Bellatrix", "Jumpgate_Betelgeuse")}, {x=120,y=0}, {Jumpgate=createObject("Jumpgate", {x=15,y=5})}),
		Jumpgate_Centauri_Six = createSite("Jumpgate - Centauri Six", "", {}, {createConnection("Centauri_Six", "Jumpgate_Betelgeuse")}, {x=-100,y=-100}, {Jumpgate=createObject("Jumpgate", {x=-15,y=-5})})
	}, { x = 20, y = -57 }),

	Bellatrix = createSystem("Bellatrix", "Known as the 'Amazon Star', Bellatrix is a massive, brilliant blue-white star. The system hosts a major naval academy and serves as a strategic checkpoint for traffic moving through the Orion sector.", {
		Jumpgate_Betelgeuse = createSite("Jumpgate - Betelgeuse", "", {}, {createConnection("Betelgeuse", "Jumpgate_Bellatrix")}, {x=-120,y=0}, {Jumpgate=createObject("Jumpgate", {x=-15,y=0})}),
		Jumpgate_Mintaka = createSite("Jumpgate - Mintaka", "", {}, {createConnection("Mintaka", "Jumpgate_Bellatrix")}, {x=0,y=-120}, {Jumpgate=createObject("Jumpgate", {x=5,y=-15})})
	}, { x = 50, y = -57 }),

	-- The Belt
	Mintaka = createSystem("Mintaka", "The westernmost star in Orion's Belt, Mintaka is a complex multiple-star system. Its intricate gravitational tides make navigation challenging, but it is a popular haven for smugglers and those wishing to avoid official patrols.", {
		Jumpgate_Bellatrix = createSite("Jumpgate - Bellatrix", "", {}, {createConnection("Bellatrix", "Jumpgate_Mintaka")}, {x=0,y=120}, {Jumpgate=createObject("Jumpgate", {x=10,y=10})}),
		Jumpgate_Alnilam = createSite("Jumpgate - Alnilam", "", {}, {createConnection("Alnilam", "Jumpgate_Mintaka")}, {x=-120,y=0}, {Jumpgate=createObject("Jumpgate", {x=-15,y=-5})})
	}, { x = 43, y = -45 }),

	Alnilam = createSystem("Alnilam", "The central star of the Belt, a blue supergiant whose intense stellar winds have scoured its inner planets clean. It is the administrative and commercial hub of the Orion sector, with vast orbital stations glittering in the star's light.", {
		Jumpgate_Mintaka = createSite("Jumpgate - Mintaka", "", {}, {createConnection("Mintaka", "Jumpgate_Alnilam")}, {x=120,y=0}, {Jumpgate=createObject("Jumpgate", {x=15,y=5})}),
		Jumpgate_Alnitak = createSite("Jumpgate - Alnitak", "", {}, {createConnection("Alnitak", "Jumpgate_Alnilam")}, {x=-120,y=0}, {Jumpgate=createObject("Jumpgate", {x=-12,y=12})}),
		Jumpgate_Orion_Nebula = createSite("Jumpgate - Orion Nebula", "", {}, {createConnection("Orion_Nebula", "Jumpgate_Alnilam")}, {x=0,y=-120}, {Jumpgate=createObject("Jumpgate", {x=0,y=-15})})
	}, { x = 35, y = -42 }),

	Alnitak = createSystem("Alnitak", "The easternmost star of the Belt. Alnitak's powerful radiation illuminates the nearby Horsehead Nebula. This system is a center for scientific research, with many outposts dedicated to studying the surrounding interstellar clouds.", {
		Jumpgate_Alnilam = createSite("Jumpgate - Alnilam", "", {}, {createConnection("Alnilam", "Jumpgate_Alnitak")}, {x=120,y=0}, {Jumpgate=createObject("Jumpgate", {x=15,y=-3})}),
		Jumpgate_Saiph = createSite("Jumpgate - Saiph", "", {}, {createConnection("Saiph", "Jumpgate_Alnitak")}, {x=0,y=-120}, {Jumpgate=createObject("Jumpgate", {x=-3,y=-15})}),
		Jumpgate_Horsehead_Nebula = createSite("Jumpgate - Horsehead Nebula", "", {}, {createConnection("Horsehead_Nebula", "Jumpgate_Alnitak")}, {x=-100,y=-100}, {Jumpgate=createObject("Jumpgate", {x=-10,y=-10})})
	}, { x = 28, y = -39 }),

	-- The Feet
	Saiph = createSystem("Saiph", "Often overlooked in favor of its brighter neighbor Rigel, Saiph is a hot, blue giant. The system is sparsely populated, home to monastic orders and colonists seeking solitude amongst its icy, distant worlds.", {
		Jumpgate_Alnitak = createSite("Jumpgate - Alnitak", "", {}, {createConnection("Alnitak", "Jumpgate_Saiph")}, {x=0,y=120}, {Jumpgate=createObject("Jumpgate", {x=0,y=15})}),
		Jumpgate_Rigel = createSite("Jumpgate - Rigel", "", {}, {createConnection("Rigel", "Jumpgate_Saiph")}, {x=120,y=0}, {Jumpgate=createObject("Jumpgate", {x=15,y=0})}),
		Jumpgate_Symbal_Ashes = createSite("Jumpgate - Symbal Ashes", "", {}, {createConnection("Symbal_Ashes", "Jumpgate_Saiph")}, {x=-100,y=100}, {Jumpgate=createObject("Jumpgate", {x=-10,y=15})})
	}, { x = 28, y = -20 }),

	Rigel = createSystem("Rigel", "A brilliant blue-white supergiant that dominates the local night sky. Rigel is a beacon for travelers and one of the most prosperous systems in the sector, famed for its orbital shipyards and vibrant trade ports.", {
		Jumpgate_Saiph = createSite("Jumpgate - Saiph", "", {}, {createConnection("Saiph", "Jumpgate_Rigel")}, {x=-120,y=0}, {Jumpgate=createObject("Jumpgate", {x=-15,y=-5})}),
		Jumpgate_Orion_Nebula = createSite("Jumpgate - Orion Nebula", "", {}, {createConnection("Orion_Nebula", "Jumpgate_Rigel")}, {x=-100,y=100}, {Jumpgate=createObject("Jumpgate", {x=-10,y=10})})
	}, { x = 50, y = -12 }),

	-- The Nebulae
	Orion_Nebula = createSystem("Orion Nebula", "A breathtaking stellar nursery where new stars are born from swirling clouds of dust and gas. The nebula is hazardous, with intense radiation and unpredictable gravimetric shears, but its core contains priceless protomatter sought by daring prospectors.", {
		Jumpgate_Alnilam = createSite("Jumpgate - Alnilam", "", {}, {createConnection("Alnilam", "Jumpgate_Orion_Nebula")}, {x=0,y=120}, {Jumpgate=createObject("Jumpgate", {x=5,y=15})}),
		Jumpgate_Rigel = createSite("Jumpgate - Rigel", "", {}, {createConnection("Rigel", "Jumpgate_Orion_Nebula")}, {x=100,y=-100}, {Jumpgate=createObject("Jumpgate", {x=15,y=-5})}),
		Jumpgate_Magnar_Solis = createSite("Jumpgate - Magnar Solis", "", {}, {createConnection("Magnar_Solis", "Jumpgate_Orion_Nebula")}, {x=0,y=120}, {Jumpgate=createObject("Jumpgate", {x=0,y=15})})
	}, { x = 35, y = -35 }),

	Horsehead_Nebula = createSystem("Horsehead Nebula", "A dark, ominous cloud of cosmic dust silhouetted against the glowing gas of IC 434. This system is largely uncharted and considered a bad omen by local spacers. Rumors persist of strange energy readings and lost ships within its murky depths.", {
		Jumpgate_Alnitak = createSite("Jumpgate - Alnitak", "", {}, {createConnection("Alnitak", "Jumpgate_Horsehead_Nebula")}, {x=100,y=100}, {Jumpgate=createObject("Jumpgate", {x=12,y=12})})
	}, { x = 26, y = -38 }),
}