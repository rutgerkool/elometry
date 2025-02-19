CREATE TABLE IF NOT EXISTS appearances (
    appearance_id TEXT, 
    game_id INT, 
    player_id INT, 
    player_club_id INT,
    player_current_club_id INT, 
    date TEXT, 
    player_name TEXT,
    competition_id TEXT, 
    yellow_cards INT, 
    red_cards INT,
    goals INT, 
    assists INT, 
    minutes_played INT
);

CREATE INDEX IF NOT EXISTS idx_appearances_game ON appearances(game_id);
CREATE INDEX IF NOT EXISTS idx_appearances_player ON appearances(player_id);

CREATE TABLE IF NOT EXISTS club_games (
    game_id INT, 
    club_id INT, 
    own_goals INT, 
    own_position TEXT,
    own_manager_name TEXT, 
    opponent_id INT, 
    opponent_goals INT,
    opponent_position TEXT, 
    opponent_manager_name TEXT,
    hosting TEXT, 
    is_win BOOLEAN
);

CREATE TABLE IF NOT EXISTS clubs (
    club_id INT,
    club_code INT,
    name TEXT,
    domestic_competition_id INT,
    total_market_value INT,
    squad_size INT,
    average_age FLOAT,
    foreigners_number INT,
    foreigners_percentage FLOAT,
    national_team_players INT,
    stadium_name TEXT,
    stadium_seats INT,
    net_transfer_record TEXT,
    coach_name TEXT,
    last_season YEAR,
    filename TEXT,
    url TEXT
);

CREATE TABLE IF NOT EXISTS competitions (
    competition_id TEXT PRIMARY KEY,
    competition_code TEXT,
    name TEXT,
    sub_type TEXT,
    type TEXT,
    country_id INT,
    country_name TEXT,
    domestic_league_code TEXT,
    confederation TEXT,
    url TEXT,
    is_major_national_league BOOLEAN
);

CREATE TABLE IF NOT EXISTS game_events (
    game_event_id TEXT,
    date DATE,
    game_id INT,
    minute INT,
    type TEXT,
    club_id INT,
    player_id INT,
    description TEXT,
    player_in_id INT,
    player_assist_id INT
);

CREATE TABLE IF NOT EXISTS game_lineups (
    game_lineups_id TEXT,
    date DATE,
    game_id INT,
    player_id INT,
    club_id INT,
    player_name TEXT,
    type TEXT,
    position TEXT,
    number INT,
    team_captain BOOLEAN
);

CREATE TABLE IF NOT EXISTS games (
    game_id INT,
    competition_id TEXT,
    season YEAR,
    round TEXT,
    date DATE,
    home_club_id INT,
    away_club_id INT,
    home_club_goals INT,
    away_club_goals INT,
    home_club_position INT,
    away_club_position INT,
    home_club_manager_name TEXT,
    away_club_manager_name TEXT,
    stadium TEXT,
    attendance INT,
    referee TEXT,
    url TEXT,
    home_club_formation TEXT,
    away_club_formation TEXT,
    home_club_name TEXT,
    away_club_name TEXT,
    aggregate TEXT,
    competition_type TEXT
);

CREATE TABLE IF NOT EXISTS player_valuations (
    player_id INT,
    date DATE,
    market_value_in_eur INT,
    current_club_id INT,
    player_club_domestic_competition_id TEXT
);

CREATE TABLE IF NOT EXISTS players (
    player_id INT PRIMARY KEY, 
    first_name TEXT, 
    last_name TEXT, 
    name TEXT,
    last_season YEAR, 
    current_club_id INT,
    player_code TEXT,
    country_of_birth TEXT,
    city_of_birth TEXT,
    country_of_citizenship TEXT,
    date_of_birth DATETIME,
    sub_position TEXT,
    position TEXT,
    foot TEXT,
    height_in_cm INT,
    contract_expiration_date DATETIME,
    agent_name TEXT,
    image_url TEXT,
    url TEXT,
    current_club_domestic_competition_id TEXT,
    current_club_name TEXT,
    market_value_in_eur INT,
    highest_market_value_in_eur INT
);

CREATE TABLE IF NOT EXISTS transfers (
    player_id INT,
    transfer_date TEXT,
    transfer_season TEXT,
    from_club_id INT,
    to_club_id INT,
    from_club_name TEXT,
    to_club_name TEXT,
    transfer_fee FLOAT,
    market_value_in_eur FLOAT,
    player_name TEXT
);
