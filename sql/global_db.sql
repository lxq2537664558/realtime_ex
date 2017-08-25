/*
MySQL Data Transfer
Source Host: localhost
Source Database: global_db
Target Host: localhost
Target Database: global_db
Date: 2017/8/22 18:44:38
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for account_base_0
-- ----------------------------
CREATE TABLE `account_base_0` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_1
-- ----------------------------
CREATE TABLE `account_base_1` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_2
-- ----------------------------
CREATE TABLE `account_base_2` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_3
-- ----------------------------
CREATE TABLE `account_base_3` (
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`account_name`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_4
-- ----------------------------
CREATE TABLE `account_base_4` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_5
-- ----------------------------
CREATE TABLE `account_base_5` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_6
-- ----------------------------
CREATE TABLE `account_base_6` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_7
-- ----------------------------
CREATE TABLE `account_base_7` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_8
-- ----------------------------
CREATE TABLE `account_base_8` (
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`account_name`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_base_9
-- ----------------------------
CREATE TABLE `account_base_9` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account_name` varchar(64) NOT NULL,
  `server_id` int(10) unsigned NOT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  `create_time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`,`server_id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;
