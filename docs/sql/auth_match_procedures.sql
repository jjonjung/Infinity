USE infinity;

DROP PROCEDURE IF EXISTS sp_user_exists_by_email;
DROP PROCEDURE IF EXISTS sp_user_exists_by_nickname;
DROP PROCEDURE IF EXISTS sp_create_local_user;
DROP PROCEDURE IF EXISTS sp_authenticate_local_user;
DROP PROCEDURE IF EXISTS sp_authenticate_social_user;
DROP PROCEDURE IF EXISTS sp_save_user_session;
DROP PROCEDURE IF EXISTS sp_upsert_match_header;
DROP PROCEDURE IF EXISTS sp_delete_match_players;
DROP PROCEDURE IF EXISTS sp_insert_match_player;
DROP PROCEDURE IF EXISTS sp_get_player_aggregate;
DROP PROCEDURE IF EXISTS sp_ensure_social_seed_user;

DELIMITER $$

CREATE PROCEDURE sp_user_exists_by_email(IN p_email VARCHAR(255))
BEGIN
    SELECT EXISTS(
        SELECT 1
        FROM users
        WHERE email = p_email
    ) AS exists_flag;
END $$

CREATE PROCEDURE sp_user_exists_by_nickname(IN p_nickname VARCHAR(50))
BEGIN
    SELECT EXISTS(
        SELECT 1
        FROM users
        WHERE nickname = p_nickname
    ) AS exists_flag;
END $$

CREATE PROCEDURE sp_create_local_user(
    IN p_email VARCHAR(255),
    IN p_nickname VARCHAR(50),
    IN p_password_hash VARCHAR(255)
)
BEGIN
    DECLARE v_user_id BIGINT;

    INSERT INTO users(email, nickname, status)
    VALUES (p_email, p_nickname, 'ACTIVE');

    SET v_user_id = LAST_INSERT_ID();

    INSERT INTO user_identities(user_id, provider, provider_user_id, password_hash, email_at_provider)
    VALUES (v_user_id, 'local', p_email, p_password_hash, p_email);

    SELECT id, email, nickname, status
    FROM users
    WHERE id = v_user_id;
END $$

CREATE PROCEDURE sp_authenticate_local_user(IN p_email VARCHAR(255))
BEGIN
    SELECT u.id, u.email, u.nickname, u.status, ui.password_hash
    FROM users u
    JOIN user_identities ui ON ui.user_id = u.id
    WHERE ui.provider = 'local'
      AND ui.provider_user_id = p_email
    LIMIT 1;
END $$

CREATE PROCEDURE sp_authenticate_social_user(
    IN p_provider VARCHAR(20),
    IN p_provider_user_id VARCHAR(191)
)
BEGIN
    SELECT u.id, u.email, u.nickname, u.status, ui.password_hash
    FROM users u
    JOIN user_identities ui ON ui.user_id = u.id
    WHERE ui.provider = p_provider
      AND ui.provider_user_id = p_provider_user_id
    LIMIT 1;
END $$

CREATE PROCEDURE sp_save_user_session(
    IN p_user_id BIGINT,
    IN p_refresh_token_hash VARCHAR(255),
    IN p_device_id VARCHAR(128),
    IN p_ip_address VARCHAR(64)
)
BEGIN
    INSERT INTO user_sessions(user_id, refresh_token_hash, device_id, ip_address, expires_at)
    VALUES (p_user_id, p_refresh_token_hash, p_device_id, p_ip_address, DATE_ADD(NOW(), INTERVAL 30 DAY));
END $$

CREATE PROCEDURE sp_upsert_match_header(
    IN p_external_match_id VARCHAR(100),
    IN p_winner_team VARCHAR(20)
)
BEGIN
    DECLARE v_match_id BIGINT DEFAULT 0;

    SELECT id
      INTO v_match_id
    FROM matches
    WHERE game_mode = 'DEDICATED'
      AND map_name = p_external_match_id
    LIMIT 1;

    IF v_match_id IS NULL OR v_match_id = 0 THEN
        INSERT INTO matches(game_mode, map_name, started_at, ended_at, winner_team, status)
        VALUES ('DEDICATED', p_external_match_id, NOW(), NOW(), p_winner_team, 'FINISHED');

        SET v_match_id = LAST_INSERT_ID();
    ELSE
        UPDATE matches
        SET winner_team = p_winner_team,
            ended_at = NOW(),
            status = 'FINISHED'
        WHERE id = v_match_id;
    END IF;

    SELECT v_match_id AS match_id;
END $$

CREATE PROCEDURE sp_delete_match_players(IN p_match_id BIGINT)
BEGIN
    DELETE FROM match_players
    WHERE match_id = p_match_id;
END $$

CREATE PROCEDURE sp_insert_match_player(
    IN p_match_id BIGINT,
    IN p_user_id BIGINT,
    IN p_team VARCHAR(20),
    IN p_character_name VARCHAR(100),
    IN p_kills INT,
    IN p_deaths INT,
    IN p_assists INT,
    IN p_damage_dealt INT,
    IN p_damage_taken INT,
    IN p_score INT,
    IN p_result VARCHAR(20)
)
BEGIN
    INSERT INTO match_players(
        match_id, user_id, team, character_name, kills, deaths, assists, damage_dealt, damage_taken, score, result
    )
    VALUES (
        p_match_id, p_user_id, p_team, p_character_name, p_kills, p_deaths, p_assists, p_damage_dealt, p_damage_taken, p_score, p_result
    );
END $$

CREATE PROCEDURE sp_get_player_aggregate(IN p_user_id BIGINT)
BEGIN
    SELECT user_id,
           COUNT(*) AS total_matches,
           SUM(CASE WHEN result = 'WIN' THEN 1 ELSE 0 END) AS total_wins,
           COALESCE(SUM(kills), 0) AS total_kills,
           COALESCE(SUM(deaths), 0) AS total_deaths,
           COALESCE(SUM(assists), 0) AS total_assists,
           COALESCE(SUM(damage_dealt), 0) AS total_damage_dealt
    FROM match_players
    WHERE user_id = p_user_id
    GROUP BY user_id;
END $$

CREATE PROCEDURE sp_ensure_social_seed_user(
    IN p_provider VARCHAR(20),
    IN p_provider_user_id VARCHAR(191),
    IN p_email VARCHAR(255),
    IN p_nickname VARCHAR(50)
)
BEGIN
    DECLARE v_user_id BIGINT DEFAULT 0;

    SELECT user_id
      INTO v_user_id
    FROM user_identities
    WHERE provider = p_provider
      AND provider_user_id = p_provider_user_id
    LIMIT 1;

    IF v_user_id IS NULL OR v_user_id = 0 THEN
        INSERT INTO users(email, nickname, status)
        VALUES (p_email, p_nickname, 'ACTIVE');

        SET v_user_id = LAST_INSERT_ID();

        INSERT INTO user_identities(user_id, provider, provider_user_id, email_at_provider)
        VALUES (v_user_id, p_provider, p_provider_user_id, p_email);
    END IF;
END $$

DELIMITER ;
