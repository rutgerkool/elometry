CREATE TABLE IF NOT EXISTS metadata (
    key TEXT PRIMARY KEY,
    value TEXT
);

CREATE TABLE IF NOT EXISTS teams (
    team_id INTEGER PRIMARY KEY,
    team_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS team_players (
    team_id INT,
    player_id INT,
    PRIMARY KEY (team_id, player_id),
    FOREIGN KEY (team_id) REFERENCES teams (team_id) ON DELETE CASCADE,
    FOREIGN KEY (player_id) REFERENCES players (player_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS team_formations (
    formation_id INTEGER PRIMARY KEY AUTOINCREMENT,
    formation_name TEXT NOT NULL,
    description TEXT
);

INSERT INTO team_formations (formation_name, description) VALUES 
('4-3-3', 'Four defenders, three midfielders, three forwards'),
('4-4-2', 'Four defenders, four midfielders, two forwards'),
('5-3-2', 'Five defenders, three midfielders, two forwards'),
('3-5-2', 'Three defenders, five midfielders, two forwards'),
('4-2-3-1', 'Four defenders, two defensive midfielders, three attacking midfielders, one forward');

CREATE TABLE IF NOT EXISTS team_lineups (
    lineup_id INTEGER PRIMARY KEY AUTOINCREMENT,
    lineup_name TEXT,
    team_id INT NOT NULL,
    formation_id INT NOT NULL,
    is_active BOOLEAN DEFAULT 1,
    FOREIGN KEY (team_id) REFERENCES teams (team_id) ON DELETE CASCADE,
    FOREIGN KEY (formation_id) REFERENCES team_formations (formation_id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS lineup_players (
    lineup_id INT NOT NULL,
    player_id INT NOT NULL,
    position_type TEXT NOT NULL,
    field_position TEXT,
    position_order INT,
    PRIMARY KEY (lineup_id, player_id),
    FOREIGN KEY (lineup_id) REFERENCES team_lineups (lineup_id) ON DELETE CASCADE,
    FOREIGN KEY (player_id) REFERENCES players (player_id) ON DELETE CASCADE
);
