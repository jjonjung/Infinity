CREATE TABLE users (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    email VARCHAR(255) NULL,
    nickname VARCHAR(50) NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'ACTIVE',
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY uq_users_email (email),
    UNIQUE KEY uq_users_nickname (nickname)
);

CREATE TABLE user_identities (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    provider VARCHAR(20) NOT NULL,
    provider_user_id VARCHAR(191) NOT NULL,
    password_hash VARCHAR(255) NULL,
    email_at_provider VARCHAR(255) NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_provider_identity (provider, provider_user_id),
    CONSTRAINT fk_user_identities_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE
);

CREATE TABLE user_sessions (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    refresh_token_hash VARCHAR(255) NOT NULL,
    device_id VARCHAR(128) NULL,
    ip_address VARCHAR(64) NULL,
    expires_at DATETIME NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    revoked_at DATETIME NULL,
    CONSTRAINT fk_user_sessions_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE
);

CREATE TABLE matches (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    game_mode VARCHAR(50) NOT NULL,
    map_name VARCHAR(100) NOT NULL,
    started_at DATETIME NOT NULL,
    ended_at DATETIME NULL,
    winner_team VARCHAR(20) NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'IN_PROGRESS',
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE match_players (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    match_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    team VARCHAR(20) NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    kills INT NOT NULL DEFAULT 0,
    deaths INT NOT NULL DEFAULT 0,
    assists INT NOT NULL DEFAULT 0,
    damage_dealt INT NOT NULL DEFAULT 0,
    damage_taken INT NOT NULL DEFAULT 0,
    score INT NOT NULL DEFAULT 0,
    result VARCHAR(20) NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_match_user (match_id, user_id),
    CONSTRAINT fk_match_players_match
        FOREIGN KEY (match_id) REFERENCES matches(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_match_players_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE
);

CREATE TABLE kill_events (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    match_id BIGINT NOT NULL,
    killer_user_id BIGINT NULL,
    victim_user_id BIGINT NOT NULL,
    killer_character_name VARCHAR(100) NULL,
    victim_character_name VARCHAR(100) NOT NULL,
    skill_tag VARCHAR(100) NULL,
    event_time_ms INT NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    KEY ix_kill_events_match_time (match_id, event_time_ms),
    CONSTRAINT fk_kill_events_match
        FOREIGN KEY (match_id) REFERENCES matches(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_kill_events_killer
        FOREIGN KEY (killer_user_id) REFERENCES users(id)
        ON DELETE SET NULL,
    CONSTRAINT fk_kill_events_victim
        FOREIGN KEY (victim_user_id) REFERENCES users(id)
        ON DELETE CASCADE
);

CREATE TABLE skill_events (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    match_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    skill_tag VARCHAR(100) NOT NULL,
    success_yn CHAR(1) NOT NULL DEFAULT 'Y',
    event_time_ms INT NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    KEY ix_skill_events_match_time (match_id, event_time_ms),
    CONSTRAINT fk_skill_events_match
        FOREIGN KEY (match_id) REFERENCES matches(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_skill_events_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE
);

CREATE TABLE player_daily_stats (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    stat_date DATE NOT NULL,
    total_matches INT NOT NULL DEFAULT 0,
    total_wins INT NOT NULL DEFAULT 0,
    total_kills INT NOT NULL DEFAULT 0,
    total_deaths INT NOT NULL DEFAULT 0,
    total_assists INT NOT NULL DEFAULT 0,
    total_damage_dealt BIGINT NOT NULL DEFAULT 0,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY uq_player_daily_stats (user_id, stat_date),
    CONSTRAINT fk_player_daily_stats_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE
);
