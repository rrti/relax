activeTest = {
	-- if false, we do *not* randomize the initial state
	-- during LEARNING episodes (for the random policies,
	-- chosen policies obviously never randomize it)
	weakBaseLine = false,

	numRandomPolicies = 10,  -- how many 'Nr' policies are learned and evaluated
	numChosenPolicies = 10,  -- how many 'Np' policies are learned and evaluated
}



parameters = {
	main = {
		-- if math is nil or these are set to some value less
		-- than zero, we use cstdlib's random() for both seeds
		initRandomSeed = (math ~= nil and math.random()) or -1,
		evalRandomSeed = (math ~= nil and math.random()) or -1,

		test = activeTest,
		data = "../data/",
	},

	learners = {
		params = {
			-- never execute more than this many actions during a LEARNING
			-- episode (very critical value: if too small, then our optimal
			-- policy may NEVER be found)
			maxActions = 10000,

			alpha = 0.1,
			gamma = 0.999999,
			epsilon = 0.333,

			alphaDecay = 1.0,
			epsilonDecay = 0.995,

			minAlpha = 0.001,
			minEpsilon = 0.001,
		},
	},

	policies = {
		-- first parameter is the number of evaluation runs we execute for
		-- a given policy (after learning it); last parameter is the action
		-- limit during each single evaluation of said policy (FIXME? if too
		-- high, avg. trial reward becomes negative)
		maxEvaluationTrials  =    100,
		maxLearningEpisodes  =  12000,
		maxEpisodeActions    =   1000,
	},

	tasks = {
		HillClimber = {
			Terrain = {
				-- acceleration in m/s^2
				gravity  = 9.81 * 0.01,
				friction = 0.9,
				stepSize = 0.01, -- dx

				-- the height-function is a parameterized cosine
				frequencyScale = 1.0,
				amplitudeScale = 2.0,
			},

			Vehicle = {
				-- these two determine acceleration in m/s^2 (via F=ma)
				mass  = 1.0,      -- kg
				force = 0.05,     -- Newton

				vmin = -3.14159,  -- max. speed along negative x-axis in m/s
				vmax =  3.14159,  -- max. speed along positive x-axis in m/s
			},

			positionMult = 100.0,
			velocityMult =  10.0,

			useRandomInitialStateActions = false, -- if true, initialize policy PI(s) randomly
			useRandomInitialActionValues = false, -- if true, initialize learner Q(s, a) randomly
		},

		SingleCorridorMaze = {
			numRows =    1,
			numCols = 1000,

			useRandomInitialStateActions = true,
			useRandomInitialActionValues = true,
		},
	},
}

return parameters

