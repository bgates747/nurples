PRAGMA foreign_keys = OFF;
BEGIN TRANSACTION;

-- -- Query to generate DROP statements for all tables
-- SELECT 'DROP TABLE IF EXISTS "' || name || '";'
-- FROM sqlite_master
-- WHERE type = 'table';

DROP TABLE IF EXISTS "tbl_00_polys_from_blender";
DROP TABLE IF EXISTS "tbl_91a_font";
DROP TABLE IF EXISTS "tbl_01_polys";
DROP TABLE IF EXISTS "tbl_02_tiles";
DROP TABLE IF EXISTS "tbl_04_panels_lookup";
DROP TABLE IF EXISTS "tbl_04a_dws_lookup";
DROP TABLE IF EXISTS "tbl_06_maps";
DROP TABLE IF EXISTS "tbl_07_render_panels";
DROP TABLE IF EXISTS "tbl_08_sfx";
DROP TABLE IF EXISTS "tbl_91b_UI";
DROP TABLE IF EXISTS "tbl_91c_UI_BJ";

COMMIT;
PRAGMA foreign_keys = ON;
