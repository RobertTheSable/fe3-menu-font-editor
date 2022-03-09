===========================================================================
==================Mystery of the Emblem Menu Font Editor===================
============================ Version 0.0.1  ===============================
============================Written by Robert==============================
===========================================================================

1. About
    This is a tileset editor for Mystery of the Emblem's menu font.
    
    This was made in about 1 day by modifying my tileset editor for FE5,
    so there are probably some bugs and/or odd behavior.
    
2. Missing Features
    This is a beta release, so several features are missing.
    1. Editing palettes - currently there is no way to edit palettes. 
    2. Loading from ROM - didn't bother to implement it yet.
    3. Expanding tilesets - may implement in the future if requested.
    
3. Basic Usage
    1. Open the tileset, tiles, and palette files 
        in File -> Open From Files. You can generate the files based on 
        info in Inserting/Extracting Files.
    2. Make your edits.
    3. Export the files via File -> Export.
    4. Insert the files into your ROM however you see fit. 
        No compression needed. More details can be found in 
        "Inserting/Extracting Files."
    
4. Questions
    1. There are some tiles at the bottom of the tileset I can't edit!
        This is intentional. The default menu tileset has an uneven number
        of tiles, so I added 5 "fake" tiles to the end to make the display
        cleaner.

5. Inserting/Extracting Files
    The program needs 3 files, and also exports the same 3 files:
    1. tileset.dat - the tileset of 8x16 glyphs used for the menu font. 
        In an unmodified ROM, this data is located at $8b9af8.
        It should be 1132 bytes long.
    2. tiles.dat - the underlying 2bpp (bits per pixel) 8x8 base tiles.
        These are combined into the 8x16 glyphs that the menu font uses. 
        The file should be 8192 bytes.
        In an unmodified ROM, this data is located at $948000.
        You can also generate it by exporting VRAM in BSNES-plus during normal
        gameplay. It's at address $A000 in VRAM.
    3. palette.bin - palettes that are in-memory when the menu is shown.
        This is the only file that can't be found in the ROM, since
        palettes are loaded from different places. You can generate it by 
        exporting CGRAM in BSNES-plus.
