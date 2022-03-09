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
        
5. Detailed usage.
    There are 3 sections of the program window to cover:
    
    1. Tileset Display - the big section on the left. This displays
    the full tileset. Clicking on a tile here will bring it into the
    active tile editor. 
    
    2. Active tile editor - Here you can edit the data of the 
    currently selected tile. The full 8x16 tile is shown in the top
    left, and you can click on the component backround tiles to bring
    one of them into focus for editing. The index of the curent background 
    tile is displayed to the right, alongside constols for the current 
    palette and vertical/horizontal flip.
    The current background tile is displayed below. Below that is the
    palette display. Here you can edit the pixels of the current 
    background tile. To change the color of a pixel, click on the desired 
    color in the palette display, then click on the pixel in the tile display
    to change its color.
    
    3. Backround Tileset Display - displays all of the background tiles 
    available in the current tileset. Double click on a tile here to set
    it as the active tile. You can also click and drag tiles onto another
    tile to copy its data.

6. Inserting/Extracting Files
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
