import random
import math
from PIL import Image

SIZE = 100
CELL_PIXELS = 25

# make grid of points
def make_grid():
    grid = []
    for x in range(SIZE):
        for y in range(SIZE):
            grid.append((x,y))

    return grid

# assign gradient vector to each point in the grid
def get_unit_vectors(grid):
    # has a random unit vector for each coordinate
    vectors = {}
    
    for coordinates in grid:
        angle = random.uniform(0, 2*math.pi)
        vector = (math.cos(angle), math.sin(angle))
        vectors[coordinates] = vector
        
    return vectors

# locate the four points of each grid cell
def locate_corners(x, y):
    points = []

    top_left = (x, y)
    top_right = (x + 1, y)
    bottom_left = (x, y + 1)
    bottom_right = (x + 1, y + 1)

    points.append(top_left)
    points.append(top_right)
    points.append(bottom_left)
    points.append(bottom_right)

    return points


def vectors_from_corners_to_point(corners, pixel_x, pixel_y):
    # sublists of this list have 4 vectors each
    vectors = []

    for corner in corners:
        vector = (pixel_x - corner[0], pixel_y - corner[1])
        vectors.append(vector)

    return vectors

# compute the dot products between unit vectors and corner 
# to point-vectors by cell (4 dot products per func call)
def compute_dot_products(unit_vectors, ctp_vectors):
    assert len(unit_vectors) == 4
    assert len(ctp_vectors) == 4

    dot_products = []
    for unit_vector, ctp_vector in zip(unit_vectors, ctp_vectors):
        dot = unit_vector[0] * ctp_vector[0] + unit_vector[1] * ctp_vector[1]
        dot_products.append(dot)

    return dot_products

# do bilinear interpolation on dot products
def interpolate(dot_products, pixel_x, pixel_y, cell_x, cell_y):
    assert len(dot_products) == 4

    x = easing_func(pixel_x - cell_x)
    y = easing_func(pixel_y - cell_y)

    m1 = dot_products[0] + x * (dot_products[1] - dot_products[0])
    m2 = dot_products[2] + x * (dot_products[3] - dot_products[2])
    m3 = m1 + y * (m2 - m1)

    return m3


def choose_unit_vectors(corners, unit_vectors):
    assert len(corners) == 4

    unit_vectors_list = []
    for corner in corners:
        unit_vectors_list.append(unit_vectors[corner])

    return unit_vectors_list


def easing_func(x):
    return 6*x ** 5 - 15*x ** 4 + 10 * x ** 3


# calculate perlin noise value at each (x, y)
def main():
    window = Image.new("RGB", (SIZE * CELL_PIXELS, SIZE * CELL_PIXELS))

    grid = make_grid()
    unit_vectors = get_unit_vectors(grid)
    
    for cell_x, cell_y in grid:
        if cell_x == SIZE - 1 or cell_y == SIZE - 1:
            continue
        corners = locate_corners(cell_x, cell_y)
        chosen_unit_vectors = choose_unit_vectors(corners, unit_vectors)

        for i in range(CELL_PIXELS):
            for j in range(CELL_PIXELS):
                x = cell_x + i / CELL_PIXELS
                y = cell_y + j / CELL_PIXELS

                ctp_vectors = vectors_from_corners_to_point(corners, x, y)
                dot_products = compute_dot_products(chosen_unit_vectors, ctp_vectors)
                
                noise_value = interpolate(dot_products, x, y, cell_x, cell_y)
                noise_value = int((noise_value + math.sqrt(2)) / (2 * math.sqrt(2)) * 255)

                window.putpixel((round(x * CELL_PIXELS), round(y * CELL_PIXELS)), (noise_value, noise_value, noise_value))
    
    window.show()

main()