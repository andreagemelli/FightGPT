from PIL import Image, ImageDraw
import random

def create_prison_background():
    # Create a larger image for better detail
    prison_size = (1200, 1200)
    prison_image = Image.new('RGBA', prison_size, (40, 40, 40))  # Dark gray base
    
    # Create a drawing context
    draw = ImageDraw.Draw(prison_image)
    
    # Draw stone blocks
    block_size = 60
    for x in range(0, prison_size[0], block_size):
        for y in range(0, prison_size[1], block_size):
            # Random variations in stone color
            stone_color = random.randint(50, 70)
            draw.rectangle([x, y, x+block_size, y+block_size], 
                         fill=(stone_color, stone_color, stone_color))
            
            # Add stone texture
            for _ in range(5):  # Add some random cracks
                crack_x = x + random.randint(0, block_size)
                crack_y = y + random.randint(0, block_size)
                crack_length = random.randint(5, 15)
                draw.line([crack_x, crack_y, crack_x + crack_length, crack_y + crack_length],
                         fill=(30, 30, 30), width=1)
    
    # Add some moss or stains
    for _ in range(20):
        stain_x = random.randint(0, prison_size[0])
        stain_y = random.randint(0, prison_size[1])
        stain_size = random.randint(20, 40)
        draw.ellipse([stain_x, stain_y, stain_x + stain_size, stain_y + stain_size],
                    fill=(30, 50, 30, 50))  # Semi-transparent green
    
    # Add some darker areas for depth
    for _ in range(10):
        dark_x = random.randint(0, prison_size[0])
        dark_y = random.randint(0, prison_size[1])
        dark_size = random.randint(50, 100)
        draw.ellipse([dark_x, dark_y, dark_x + dark_size, dark_y + dark_size],
                    fill=(20, 20, 20, 100))  # Semi-transparent black
    
    return prison_image

def create_wall_texture():
    wall_size = (40, 40)
    wall_image = Image.new('RGBA', wall_size, (60, 60, 60))  # Dark gray base
    draw = ImageDraw.Draw(wall_image)
    
    # Draw brick pattern
    brick_width = 20
    brick_height = 10
    for x in range(0, wall_size[0], brick_width):
        for y in range(0, wall_size[1], brick_height):
            # Random variations in brick color
            brick_color = random.randint(50, 70)
            draw.rectangle([x, y, x+brick_width-2, y+brick_height-2],
                         fill=(brick_color, brick_color, brick_color))
            
            # Add mortar lines
            draw.line([x, y, x+brick_width-2, y], fill=(40, 40, 40))
            draw.line([x, y, x, y+brick_height-2], fill=(40, 40, 40))
    
    # Add some texture
    for _ in range(5):
        crack_x = random.randint(0, wall_size[0])
        crack_y = random.randint(0, wall_size[1])
        crack_length = random.randint(5, 10)
        draw.line([crack_x, crack_y, crack_x + crack_length, crack_y + crack_length],
                 fill=(30, 30, 30), width=1)
    
    return wall_image

# Create and save the textures
prison_image = create_prison_background()
prison_image.save('assets/maps/prison.png')

wall_image = create_wall_texture()
wall_image.save('assets/maps/wall.png') 