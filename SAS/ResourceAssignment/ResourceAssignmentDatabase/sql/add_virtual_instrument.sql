-- This file was generated by the 'create_add_virtual_instrument.sql.py' script.
-- Please do not modify this sql file, but modify and the python script if you want a different virtual instrument.

-- resourceassignment password for testing on mcu005 is the same as the password on the president's luggage +6
-- psql resourceassignment -U resourceassignment -f add_virtual_instrument.sql -W
BEGIN;
-- start with clearing the old virtual instrument model
-- for some reason truncate takes a looooong time to complete, so use delete from
DELETE FROM virtual_instrument.resource_group_to_resource_group CASCADE;
DELETE FROM virtual_instrument.resource_to_resource_group CASCADE;
DELETE FROM virtual_instrument.resource_group CASCADE;
DELETE FROM virtual_instrument.resource_group_type CASCADE;
DELETE FROM virtual_instrument.resource CASCADE;
DELETE FROM virtual_instrument.resource_type CASCADE;
DELETE FROM virtual_instrument.unit CASCADE;
-- end of initial cleanup

INSERT INTO virtual_instrument.unit VALUES 
(0, 'rsp_channel_bit'),(1, 'bytes'),(2, 'rcu_board'),(3, 'bytes/second'),(4, 'cores');
INSERT INTO virtual_instrument.resource_type VALUES 
(0, 'rsp', 0), (1, 'tbb', 1), (2, 'rcu', 2), (3, 'bandwidth', 3), (4, 'processor', 4), (5, 'storage', 1);
INSERT INTO virtual_instrument.resource_group_type VALUES 
(0, 'instrument'),(1, 'cluster'),(2, 'station_group'),(3, 'station'),(4, 'node_group'),(5, 'node');
INSERT INTO virtual_instrument.resource_group VALUES 
(0, 'LOFAR', 0), (1, 'CEP4', 1),
(2, 'Cobalt', 1), (3, 'computenodes', 4),
(4, 'gpunodes', 4), (5, 'cpunode01', 5),
(6, 'cpunode02', 5),
(7, 'cpunode03', 5),
(8, 'cpunode04', 5),
(9, 'cpunode05', 5),
(10, 'cpunode06', 5),
(11, 'cpunode07', 5),
(12, 'cpunode08', 5),
(13, 'cpunode09', 5),
(14, 'cpunode10', 5),
(15, 'cpunode11', 5),
(16, 'cpunode12', 5),
(17, 'cpunode13', 5),
(18, 'cpunode14', 5),
(19, 'cpunode15', 5),
(20, 'cpunode16', 5),
(21, 'cpunode17', 5),
(22, 'cpunode18', 5),
(23, 'cpunode19', 5),
(24, 'cpunode20', 5),
(25, 'cpunode21', 5),
(26, 'cpunode22', 5),
(27, 'cpunode23', 5),
(28, 'cpunode24', 5),
(29, 'cpunode25', 5),
(30, 'cpunode26', 5),
(31, 'cpunode27', 5),
(32, 'cpunode28', 5),
(33, 'cpunode29', 5),
(34, 'cpunode30', 5),
(35, 'cpunode31', 5),
(36, 'cpunode32', 5),
(37, 'cpunode33', 5),
(38, 'cpunode34', 5),
(39, 'cpunode35', 5),
(40, 'cpunode36', 5),
(41, 'cpunode37', 5),
(42, 'cpunode38', 5),
(43, 'cpunode39', 5),
(44, 'cpunode40', 5),
(45, 'cpunode41', 5),
(46, 'cpunode42', 5),
(47, 'cpunode43', 5),
(48, 'cpunode44', 5),
(49, 'cpunode45', 5),
(50, 'cpunode46', 5),
(51, 'cpunode47', 5),
(52, 'cpunode48', 5),
(53, 'cpunode49', 5),
(54, 'cpunode50', 5), (55, 'cbt001', 5),
(56, 'cbt002', 5),
(57, 'cbt003', 5),
(58, 'cbt004', 5),
(59, 'cbt005', 5),
(60, 'cbt006', 5),
(61, 'cbt007', 5),
(62, 'cbt008', 5);
INSERT INTO virtual_instrument.resource VALUES (0, 'bandwidth', 3), (1, 'processor', 4), (2, 'bandwidth', 3), (3, 'processor', 4), (4, 'bandwidth', 3), (5, 'processor', 4), (6, 'bandwidth', 3), (7, 'processor', 4), (8, 'bandwidth', 3), (9, 'processor', 4), (10, 'bandwidth', 3), (11, 'processor', 4), (12, 'bandwidth', 3), (13, 'processor', 4), (14, 'bandwidth', 3), (15, 'processor', 4), (16, 'bandwidth', 3), (17, 'processor', 4), (18, 'bandwidth', 3), (19, 'processor', 4), (20, 'bandwidth', 3), (21, 'processor', 4), (22, 'bandwidth', 3), (23, 'processor', 4), (24, 'bandwidth', 3), (25, 'processor', 4), (26, 'bandwidth', 3), (27, 'processor', 4), (28, 'bandwidth', 3), (29, 'processor', 4), (30, 'bandwidth', 3), (31, 'processor', 4), (32, 'bandwidth', 3), (33, 'processor', 4), (34, 'bandwidth', 3), (35, 'processor', 4), (36, 'bandwidth', 3), (37, 'processor', 4), (38, 'bandwidth', 3), (39, 'processor', 4), (40, 'bandwidth', 3), (41, 'processor', 4), (42, 'bandwidth', 3), (43, 'processor', 4), (44, 'bandwidth', 3), (45, 'processor', 4), (46, 'bandwidth', 3), (47, 'processor', 4), (48, 'bandwidth', 3), (49, 'processor', 4), (50, 'bandwidth', 3), (51, 'processor', 4), (52, 'bandwidth', 3), (53, 'processor', 4), (54, 'bandwidth', 3), (55, 'processor', 4), (56, 'bandwidth', 3), (57, 'processor', 4), (58, 'bandwidth', 3), (59, 'processor', 4), (60, 'bandwidth', 3), (61, 'processor', 4), (62, 'bandwidth', 3), (63, 'processor', 4), (64, 'bandwidth', 3), (65, 'processor', 4), (66, 'bandwidth', 3), (67, 'processor', 4), (68, 'bandwidth', 3), (69, 'processor', 4), (70, 'bandwidth', 3), (71, 'processor', 4), (72, 'bandwidth', 3), (73, 'processor', 4), (74, 'bandwidth', 3), (75, 'processor', 4), (76, 'bandwidth', 3), (77, 'processor', 4), (78, 'bandwidth', 3), (79, 'processor', 4), (80, 'bandwidth', 3), (81, 'processor', 4), (82, 'bandwidth', 3), (83, 'processor', 4), (84, 'bandwidth', 3), (85, 'processor', 4), (86, 'bandwidth', 3), (87, 'processor', 4), (88, 'bandwidth', 3), (89, 'processor', 4), (90, 'bandwidth', 3), (91, 'processor', 4), (92, 'bandwidth', 3), (93, 'processor', 4), (94, 'bandwidth', 3), (95, 'processor', 4), (96, 'bandwidth', 3), (97, 'processor', 4), (98, 'bandwidth', 3), (99, 'processor', 4), (100, 'bandwidth', 3), (101, 'processor', 4), (102, 'bandwidth', 3), (103, 'processor', 4), (104, 'bandwidth', 3), (105, 'processor', 4), (106, 'bandwidth', 3), (107, 'processor', 4), (108, 'bandwidth', 3), (109, 'processor', 4), (110, 'bandwidth', 3), (111, 'processor', 4), (112, 'bandwidth', 3), (113, 'processor', 4), (114, 'bandwidth', 3), (115, 'processor', 4), (116, 'cep4bandwidth', 3), (117, 'cep4storage', 5);
INSERT INTO virtual_instrument.resource_to_resource_group VALUES (DEFAULT, 0, 5), (DEFAULT, 1, 5), (DEFAULT, 2, 6), (DEFAULT, 3, 6), (DEFAULT, 4, 7), (DEFAULT, 5, 7), (DEFAULT, 6, 8), (DEFAULT, 7, 8), (DEFAULT, 8, 9), (DEFAULT, 9, 9), (DEFAULT, 10, 10), (DEFAULT, 11, 10), (DEFAULT, 12, 11), (DEFAULT, 13, 11), (DEFAULT, 14, 12), (DEFAULT, 15, 12), (DEFAULT, 16, 13), (DEFAULT, 17, 13), (DEFAULT, 18, 14), (DEFAULT, 19, 14), (DEFAULT, 20, 15), (DEFAULT, 21, 15), (DEFAULT, 22, 16), (DEFAULT, 23, 16), (DEFAULT, 24, 17), (DEFAULT, 25, 17), (DEFAULT, 26, 18), (DEFAULT, 27, 18), (DEFAULT, 28, 19), (DEFAULT, 29, 19), (DEFAULT, 30, 20), (DEFAULT, 31, 20), (DEFAULT, 32, 21), (DEFAULT, 33, 21), (DEFAULT, 34, 22), (DEFAULT, 35, 22), (DEFAULT, 36, 23), (DEFAULT, 37, 23), (DEFAULT, 38, 24), (DEFAULT, 39, 24), (DEFAULT, 40, 25), (DEFAULT, 41, 25), (DEFAULT, 42, 26), (DEFAULT, 43, 26), (DEFAULT, 44, 27), (DEFAULT, 45, 27), (DEFAULT, 46, 28), (DEFAULT, 47, 28), (DEFAULT, 48, 29), (DEFAULT, 49, 29), (DEFAULT, 50, 30), (DEFAULT, 51, 30), (DEFAULT, 52, 31), (DEFAULT, 53, 31), (DEFAULT, 54, 32), (DEFAULT, 55, 32), (DEFAULT, 56, 33), (DEFAULT, 57, 33), (DEFAULT, 58, 34), (DEFAULT, 59, 34), (DEFAULT, 60, 35), (DEFAULT, 61, 35), (DEFAULT, 62, 36), (DEFAULT, 63, 36), (DEFAULT, 64, 37), (DEFAULT, 65, 37), (DEFAULT, 66, 38), (DEFAULT, 67, 38), (DEFAULT, 68, 39), (DEFAULT, 69, 39), (DEFAULT, 70, 40), (DEFAULT, 71, 40), (DEFAULT, 72, 41), (DEFAULT, 73, 41), (DEFAULT, 74, 42), (DEFAULT, 75, 42), (DEFAULT, 76, 43), (DEFAULT, 77, 43), (DEFAULT, 78, 44), (DEFAULT, 79, 44), (DEFAULT, 80, 45), (DEFAULT, 81, 45), (DEFAULT, 82, 46), (DEFAULT, 83, 46), (DEFAULT, 84, 47), (DEFAULT, 85, 47), (DEFAULT, 86, 48), (DEFAULT, 87, 48), (DEFAULT, 88, 49), (DEFAULT, 89, 49), (DEFAULT, 90, 50), (DEFAULT, 91, 50), (DEFAULT, 92, 51), (DEFAULT, 93, 51), (DEFAULT, 94, 52), (DEFAULT, 95, 52), (DEFAULT, 96, 53), (DEFAULT, 97, 53), (DEFAULT, 98, 54), (DEFAULT, 99, 54), (DEFAULT, 100, 55), (DEFAULT, 101, 55), (DEFAULT, 102, 56), (DEFAULT, 103, 56), (DEFAULT, 104, 57), (DEFAULT, 105, 57), (DEFAULT, 106, 58), (DEFAULT, 107, 58), (DEFAULT, 108, 59), (DEFAULT, 109, 59), (DEFAULT, 110, 60), (DEFAULT, 111, 60), (DEFAULT, 112, 61), (DEFAULT, 113, 61), (DEFAULT, 114, 62), (DEFAULT, 115, 62), (DEFAULT, 116, 1), (DEFAULT, 117, 1);
INSERT INTO resource_monitoring.resource_capacity VALUES (DEFAULT, 0, 53687091200, 53687091200), (DEFAULT, 1, 24, 24), (DEFAULT, 2, 53687091200, 53687091200), (DEFAULT, 3, 24, 24), (DEFAULT, 4, 53687091200, 53687091200), (DEFAULT, 5, 24, 24), (DEFAULT, 6, 53687091200, 53687091200), (DEFAULT, 7, 24, 24), (DEFAULT, 8, 53687091200, 53687091200), (DEFAULT, 9, 24, 24), (DEFAULT, 10, 53687091200, 53687091200), (DEFAULT, 11, 24, 24), (DEFAULT, 12, 53687091200, 53687091200), (DEFAULT, 13, 24, 24), (DEFAULT, 14, 53687091200, 53687091200), (DEFAULT, 15, 24, 24), (DEFAULT, 16, 53687091200, 53687091200), (DEFAULT, 17, 24, 24), (DEFAULT, 18, 53687091200, 53687091200), (DEFAULT, 19, 24, 24), (DEFAULT, 20, 53687091200, 53687091200), (DEFAULT, 21, 24, 24), (DEFAULT, 22, 53687091200, 53687091200), (DEFAULT, 23, 24, 24), (DEFAULT, 24, 53687091200, 53687091200), (DEFAULT, 25, 24, 24), (DEFAULT, 26, 53687091200, 53687091200), (DEFAULT, 27, 24, 24), (DEFAULT, 28, 53687091200, 53687091200), (DEFAULT, 29, 24, 24), (DEFAULT, 30, 53687091200, 53687091200), (DEFAULT, 31, 24, 24), (DEFAULT, 32, 53687091200, 53687091200), (DEFAULT, 33, 24, 24), (DEFAULT, 34, 53687091200, 53687091200), (DEFAULT, 35, 24, 24), (DEFAULT, 36, 53687091200, 53687091200), (DEFAULT, 37, 24, 24), (DEFAULT, 38, 53687091200, 53687091200), (DEFAULT, 39, 24, 24), (DEFAULT, 40, 53687091200, 53687091200), (DEFAULT, 41, 24, 24), (DEFAULT, 42, 53687091200, 53687091200), (DEFAULT, 43, 24, 24), (DEFAULT, 44, 53687091200, 53687091200), (DEFAULT, 45, 24, 24), (DEFAULT, 46, 53687091200, 53687091200), (DEFAULT, 47, 24, 24), (DEFAULT, 48, 53687091200, 53687091200), (DEFAULT, 49, 24, 24), (DEFAULT, 50, 53687091200, 53687091200), (DEFAULT, 51, 24, 24), (DEFAULT, 52, 53687091200, 53687091200), (DEFAULT, 53, 24, 24), (DEFAULT, 54, 53687091200, 53687091200), (DEFAULT, 55, 24, 24), (DEFAULT, 56, 53687091200, 53687091200), (DEFAULT, 57, 24, 24), (DEFAULT, 58, 53687091200, 53687091200), (DEFAULT, 59, 24, 24), (DEFAULT, 60, 53687091200, 53687091200), (DEFAULT, 61, 24, 24), (DEFAULT, 62, 53687091200, 53687091200), (DEFAULT, 63, 24, 24), (DEFAULT, 64, 53687091200, 53687091200), (DEFAULT, 65, 24, 24), (DEFAULT, 66, 53687091200, 53687091200), (DEFAULT, 67, 24, 24), (DEFAULT, 68, 53687091200, 53687091200), (DEFAULT, 69, 24, 24), (DEFAULT, 70, 53687091200, 53687091200), (DEFAULT, 71, 24, 24), (DEFAULT, 72, 53687091200, 53687091200), (DEFAULT, 73, 24, 24), (DEFAULT, 74, 53687091200, 53687091200), (DEFAULT, 75, 24, 24), (DEFAULT, 76, 53687091200, 53687091200), (DEFAULT, 77, 24, 24), (DEFAULT, 78, 53687091200, 53687091200), (DEFAULT, 79, 24, 24), (DEFAULT, 80, 53687091200, 53687091200), (DEFAULT, 81, 24, 24), (DEFAULT, 82, 53687091200, 53687091200), (DEFAULT, 83, 24, 24), (DEFAULT, 84, 53687091200, 53687091200), (DEFAULT, 85, 24, 24), (DEFAULT, 86, 53687091200, 53687091200), (DEFAULT, 87, 24, 24), (DEFAULT, 88, 53687091200, 53687091200), (DEFAULT, 89, 24, 24), (DEFAULT, 90, 53687091200, 53687091200), (DEFAULT, 91, 24, 24), (DEFAULT, 92, 53687091200, 53687091200), (DEFAULT, 93, 24, 24), (DEFAULT, 94, 53687091200, 53687091200), (DEFAULT, 95, 24, 24), (DEFAULT, 96, 53687091200, 53687091200), (DEFAULT, 97, 24, 24), (DEFAULT, 98, 53687091200, 53687091200), (DEFAULT, 99, 24, 24), (DEFAULT, 100, 53687091200, 53687091200), (DEFAULT, 101, 24, 24), (DEFAULT, 102, 53687091200, 53687091200), (DEFAULT, 103, 24, 24), (DEFAULT, 104, 53687091200, 53687091200), (DEFAULT, 105, 24, 24), (DEFAULT, 106, 53687091200, 53687091200), (DEFAULT, 107, 24, 24), (DEFAULT, 108, 53687091200, 53687091200), (DEFAULT, 109, 24, 24), (DEFAULT, 110, 53687091200, 53687091200), (DEFAULT, 111, 24, 24), (DEFAULT, 112, 53687091200, 53687091200), (DEFAULT, 113, 24, 24), (DEFAULT, 114, 53687091200, 53687091200), (DEFAULT, 115, 24, 24), (DEFAULT, 116, 85899345920, 85899345920), (DEFAULT, 117, 2254857830400, 2254857830400);
INSERT INTO resource_monitoring.resource_availability VALUES (DEFAULT, 0, TRUE), (DEFAULT, 1, TRUE), (DEFAULT, 2, TRUE), (DEFAULT, 3, TRUE), (DEFAULT, 4, TRUE), (DEFAULT, 5, TRUE), (DEFAULT, 6, TRUE), (DEFAULT, 7, TRUE), (DEFAULT, 8, TRUE), (DEFAULT, 9, TRUE), (DEFAULT, 10, TRUE), (DEFAULT, 11, TRUE), (DEFAULT, 12, TRUE), (DEFAULT, 13, TRUE), (DEFAULT, 14, TRUE), (DEFAULT, 15, TRUE), (DEFAULT, 16, TRUE), (DEFAULT, 17, TRUE), (DEFAULT, 18, TRUE), (DEFAULT, 19, TRUE), (DEFAULT, 20, TRUE), (DEFAULT, 21, TRUE), (DEFAULT, 22, TRUE), (DEFAULT, 23, TRUE), (DEFAULT, 24, TRUE), (DEFAULT, 25, TRUE), (DEFAULT, 26, TRUE), (DEFAULT, 27, TRUE), (DEFAULT, 28, TRUE), (DEFAULT, 29, TRUE), (DEFAULT, 30, TRUE), (DEFAULT, 31, TRUE), (DEFAULT, 32, TRUE), (DEFAULT, 33, TRUE), (DEFAULT, 34, TRUE), (DEFAULT, 35, TRUE), (DEFAULT, 36, TRUE), (DEFAULT, 37, TRUE), (DEFAULT, 38, TRUE), (DEFAULT, 39, TRUE), (DEFAULT, 40, TRUE), (DEFAULT, 41, TRUE), (DEFAULT, 42, TRUE), (DEFAULT, 43, TRUE), (DEFAULT, 44, TRUE), (DEFAULT, 45, TRUE), (DEFAULT, 46, TRUE), (DEFAULT, 47, TRUE), (DEFAULT, 48, TRUE), (DEFAULT, 49, TRUE), (DEFAULT, 50, TRUE), (DEFAULT, 51, TRUE), (DEFAULT, 52, TRUE), (DEFAULT, 53, TRUE), (DEFAULT, 54, TRUE), (DEFAULT, 55, TRUE), (DEFAULT, 56, TRUE), (DEFAULT, 57, TRUE), (DEFAULT, 58, TRUE), (DEFAULT, 59, TRUE), (DEFAULT, 60, TRUE), (DEFAULT, 61, TRUE), (DEFAULT, 62, TRUE), (DEFAULT, 63, TRUE), (DEFAULT, 64, TRUE), (DEFAULT, 65, TRUE), (DEFAULT, 66, TRUE), (DEFAULT, 67, TRUE), (DEFAULT, 68, TRUE), (DEFAULT, 69, TRUE), (DEFAULT, 70, TRUE), (DEFAULT, 71, TRUE), (DEFAULT, 72, TRUE), (DEFAULT, 73, TRUE), (DEFAULT, 74, TRUE), (DEFAULT, 75, TRUE), (DEFAULT, 76, TRUE), (DEFAULT, 77, TRUE), (DEFAULT, 78, TRUE), (DEFAULT, 79, TRUE), (DEFAULT, 80, TRUE), (DEFAULT, 81, TRUE), (DEFAULT, 82, TRUE), (DEFAULT, 83, TRUE), (DEFAULT, 84, TRUE), (DEFAULT, 85, TRUE), (DEFAULT, 86, TRUE), (DEFAULT, 87, TRUE), (DEFAULT, 88, TRUE), (DEFAULT, 89, TRUE), (DEFAULT, 90, TRUE), (DEFAULT, 91, TRUE), (DEFAULT, 92, TRUE), (DEFAULT, 93, TRUE), (DEFAULT, 94, TRUE), (DEFAULT, 95, TRUE), (DEFAULT, 96, TRUE), (DEFAULT, 97, TRUE), (DEFAULT, 98, TRUE), (DEFAULT, 99, TRUE), (DEFAULT, 100, TRUE), (DEFAULT, 101, TRUE), (DEFAULT, 102, TRUE), (DEFAULT, 103, TRUE), (DEFAULT, 104, TRUE), (DEFAULT, 105, TRUE), (DEFAULT, 106, TRUE), (DEFAULT, 107, TRUE), (DEFAULT, 108, TRUE), (DEFAULT, 109, TRUE), (DEFAULT, 110, TRUE), (DEFAULT, 111, TRUE), (DEFAULT, 112, TRUE), (DEFAULT, 113, TRUE), (DEFAULT, 114, TRUE), (DEFAULT, 115, TRUE), (DEFAULT, 116, TRUE), (DEFAULT, 117, TRUE);
INSERT INTO virtual_instrument.resource_group_to_resource_group VALUES (DEFAULT, 0, NULL), (DEFAULT, 3, 1), (DEFAULT, 5, 3), (DEFAULT, 6, 3), (DEFAULT, 7, 3), (DEFAULT, 8, 3), (DEFAULT, 9, 3), (DEFAULT, 10, 3), (DEFAULT, 11, 3), (DEFAULT, 12, 3), (DEFAULT, 13, 3), (DEFAULT, 14, 3), (DEFAULT, 15, 3), (DEFAULT, 16, 3), (DEFAULT, 17, 3), (DEFAULT, 18, 3), (DEFAULT, 19, 3), (DEFAULT, 20, 3), (DEFAULT, 21, 3), (DEFAULT, 22, 3), (DEFAULT, 23, 3), (DEFAULT, 24, 3), (DEFAULT, 25, 3), (DEFAULT, 26, 3), (DEFAULT, 27, 3), (DEFAULT, 28, 3), (DEFAULT, 29, 3), (DEFAULT, 30, 3), (DEFAULT, 31, 3), (DEFAULT, 32, 3), (DEFAULT, 33, 3), (DEFAULT, 34, 3), (DEFAULT, 35, 3), (DEFAULT, 36, 3), (DEFAULT, 37, 3), (DEFAULT, 38, 3), (DEFAULT, 39, 3), (DEFAULT, 40, 3), (DEFAULT, 41, 3), (DEFAULT, 42, 3), (DEFAULT, 43, 3), (DEFAULT, 44, 3), (DEFAULT, 45, 3), (DEFAULT, 46, 3), (DEFAULT, 47, 3), (DEFAULT, 48, 3), (DEFAULT, 49, 3), (DEFAULT, 50, 3), (DEFAULT, 51, 3), (DEFAULT, 52, 3), (DEFAULT, 53, 3), (DEFAULT, 54, 3), (DEFAULT, 55, 2), (DEFAULT, 56, 2), (DEFAULT, 57, 2), (DEFAULT, 58, 2), (DEFAULT, 59, 2), (DEFAULT, 60, 2), (DEFAULT, 61, 2), (DEFAULT, 62, 2), (DEFAULT, 1, 0), (DEFAULT, 2, 0);
COMMIT;