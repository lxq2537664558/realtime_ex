/*
MySQL Data Transfer
Source Host: localhost
Source Database: game_db
Target Host: localhost
Target Database: game_db
Date: 2017/8/22 18:44:43
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for player_base
-- ----------------------------
CREATE TABLE `player_base` (
  `player_id` bigint(20) unsigned NOT NULL,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `last_login_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `last_logout_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL DEFAULT '',
  `lv` int(10) unsigned NOT NULL DEFAULT '1',
  `gold` bigint(20) unsigned NOT NULL DEFAULT '0',
  `money` bigint(20) unsigned NOT NULL DEFAULT '0',
  `vitality` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`player_id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for player_id_gen
-- ----------------------------
CREATE TABLE `player_id_gen` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Procedure structure for sp_create_player_base
-- ----------------------------
DELIMITER ;;
CREATE DEFINER=`root`@`%` PROCEDURE `sp_create_player_base`(in p_account_name varchar(64), in p_server_id int, in p_db_id int)
BEGIN
	declare tmp_id bigint default 0;
	declare tmp_player_id bigint default 0;

	start transaction;
	select player_id into @tmp_id from player_base where account_name= p_account_name and server_id = p_server_id limit 1;
	if FOUND_ROWS() = 0 then
		insert into player_id_gen values();

		select last_insert_id() into @tmp_id;
		set @tmp_id = @tmp_id|p_db_id<<54;

		insert into player_base(player_id, account_name, server_id) values(@tmp_id, p_account_name, p_server_id);
		if FOUND_ROWS() = 0 then
			set @tmp_player_id = @tmp_id;
		end if;
	else
		set @tmp_player_id = @tmp_id;
	end if;

	select @tmp_player_id;
	commit;
END;;
DELIMITER ;
