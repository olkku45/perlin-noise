#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define CELL_AMOUNT 40
#define HEIGHT 800
#define WIDTH 800
#define CELL_PIXELS (WIDTH / CELL_AMOUNT)

typedef struct {
	float x;
	float y;
} Vec2;

typedef struct {
	Vec2 arr[(CELL_AMOUNT + 1) * (CELL_AMOUNT + 1)];
} VectorArray;

typedef struct {
	Vec2 arr[4];
} VectorArray4;

typedef struct {
	float arr[4];
} FloatArray4;

typedef struct {
	float arr[HEIGHT][WIDTH];
} NoiseArray;


double randomUniform(double min, double max) {
    double rn = (double)rand() / RAND_MAX;
    return min + rn * (max - min);
}

VectorArray* getUnitVectors() {
	VectorArray* vectors = malloc(sizeof(VectorArray));
	int gridSize = (CELL_AMOUNT + 1) * (CELL_AMOUNT + 1);

	for (int i = 0; i < gridSize; i++) {
		double angle = randomUniform(0.0, 6.28);
		vectors->arr[i].x = cosf(angle);
		vectors->arr[i].y = sinf(angle);
	}
	return vectors;
}

VectorArray4* locateCellCorners(int x, int y) {
	VectorArray4* corners = malloc(sizeof(VectorArray4));

	Vec2 topLeft = {x, y};
	Vec2 topRight = {x + 1, y};
	Vec2 bottomLeft = {x, y + 1};
	Vec2 bottomRight = {x + 1, y + 1};

	corners->arr[0] = topLeft;
	corners->arr[1] = topRight;
	corners->arr[2] = bottomLeft;
	corners->arr[3] = bottomRight;

	return corners;
}

VectorArray4* ctpVectors(VectorArray4* corners, float pixelX, float pixelY) {
	VectorArray4* ctpVectorsArr = malloc(sizeof(VectorArray4));

	for (int i = 0; i < 4; i++) {
		Vec2 vector;
		vector.x = pixelX - corners->arr[i].x;
		vector.y = pixelY - corners->arr[i].y;
		ctpVectorsArr->arr[i] = vector;
	}
	return ctpVectorsArr;
}

FloatArray4* getDotProducts(VectorArray4* unitVectors, VectorArray4* ctpVectors) {
	FloatArray4* dotProducts = malloc(sizeof(VectorArray4));

	for (int i = 0; i < 4; i++) {
		Vec2 unitVector = unitVectors->arr[i];
		Vec2 ctpVector = ctpVectors->arr[i];

		float dot = unitVector.x * ctpVector.x + unitVector.y * ctpVector.y;
		dotProducts->arr[i] = dot;
	}
	return dotProducts;
}

float easingFunc(float x) {
	return 6*pow(x, 5) - 15*pow(x, 4) + 10*pow(x, 3);
}

float interpolate(FloatArray4* dotProducts, float pixelX, float pixelY, int cellX, int cellY) {
	float x = easingFunc(pixelX - cellX);
	float y = easingFunc(pixelY - cellY);

	float i1 = dotProducts->arr[0] + x * (dotProducts->arr[1] - dotProducts->arr[0]);
	float i2 = dotProducts->arr[2] + x * (dotProducts->arr[3] - dotProducts->arr[2]);
	float result = i1 + y * (i2 - i1);

	return result;
}

VectorArray4* chooseUnitVectors(VectorArray4* corners, VectorArray* unitVectors) {
	VectorArray4* chosenVecArr = malloc(sizeof(VectorArray4));

	for (int i = 0; i < 4; i++) {
		Vec2 corner = corners->arr[i];

		int gridIndex = (int)corner.y * (CELL_AMOUNT + 1) + (int)corner.x;

		chosenVecArr->arr[i] = unitVectors->arr[gridIndex];
	}
	return chosenVecArr;
}

void savePerlinImage(NoiseArray* noise, int width, int height, const char* filename) {
    FILE* file = fopen(filename, "wb");
    fprintf(file, "P6\n%d %d\n255\n", width, height);

    float minValue = -sqrt(2.0) / 2.0;
    float maxValue = sqrt(2.0) / 2.0;
    float range = maxValue - minValue;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float normalized = (noise->arr[y][x] - minValue) / range;

            unsigned char gray = (unsigned char)(normalized * 255.0);

            fputc(gray, file);
            fputc(gray, file);
            fputc(gray, file);
        }
    }
    fclose(file);
}

int main() {
	srand(time(NULL));
	VectorArray* unitVectors = getUnitVectors();
	NoiseArray* noise = malloc(sizeof(NoiseArray));

	for (int cellX = 0; cellX < CELL_AMOUNT; cellX++) {
		for (int cellY = 0; cellY < CELL_AMOUNT; cellY++) {

			if (cellX == CELL_AMOUNT || cellY == CELL_AMOUNT) {
				continue;
			}

			VectorArray4* cellCorners = locateCellCorners(cellX, cellY);
			VectorArray4* chosenUnitVectors = chooseUnitVectors(cellCorners, unitVectors);

			for (int i = 0; i < CELL_PIXELS; i++) {
				for (int j = 0; j < CELL_PIXELS; j++) {
					float pixelX = (float)cellX + (float)i / (float)CELL_PIXELS;
					float pixelY = (float)cellY + (float)j / (float)CELL_PIXELS;

					VectorArray4* ctpVectorsArr = ctpVectors(cellCorners, pixelX, pixelY);
					FloatArray4* dotProductsArr = getDotProducts(chosenUnitVectors, ctpVectorsArr);

					float noiseValue = interpolate(dotProductsArr, pixelX, pixelY, cellX, cellY);
					float pixelValue = roundf((noiseValue + sqrtf(2) / 2) / sqrtf(2) * 255);

					noise->arr[(int)(pixelY * CELL_PIXELS)][(int)(pixelX * CELL_PIXELS)] = noiseValue;

					free(ctpVectorsArr);
					free(dotProductsArr);
				}
			}
			free(cellCorners);
			free(chosenUnitVectors);
		}
	}
	free(unitVectors);
	savePerlinImage(noise, WIDTH, HEIGHT, "thingy.ppm");
	free(noise);

	return 0;
}