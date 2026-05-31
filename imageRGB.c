/// imageRGB - A simple image module for handling RGB images,
///            pixel color values are represented using a look-up table (LUT)
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
/// 2025

// Student authors (fill in below):
// NMec: 125087
// Name: Henrique Tardelli
// NMec: 126180
// Name: Gabriel Gúzman
//
// Date: 14-11-2025
//

#include "imageRGB.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PixelCoords.h"
#include "PixelCoordsQueue.h"
#include "PixelCoordsStack.h"
#include "instrumentation.h"

// The data structure
//
// A RGB image is stored in a structure containing 5 fields:
// Two integers store the image width and height.
// The next field is a pointer to an array that stores the pointers
// to the image rows.
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// FIXED SIZE of LUT for storing RGB triplets
#define FIXED_LUT_SIZE 1000

// Internal structure for storing RGB images
struct image {
  uint32 width;
  uint32 height;
  uint16** image;     // pointer to an array of pointers referencing the image rows
  uint16 num_colors;  // the number of colors (i.e., pixel labels) used
  rgb_t* LUT;         // table storing (R,G,B) triplets
};

// Design by Contract

// This module follows "design-by-contract" principles.
// `assert` is used to check function preconditions, postconditions
// and type invariants.
// This helps to find programmer errors.

/// Defensive Error Handling

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
//
// When one of these functions detects a memory or I/O error,
// it immediately prints an error message and aborts the program.
// This is a Fail-Fast strategy.
//
// You may use the `check` function to check a condition
// and exit the program with an error message if it is false.
// Note that it works similarly to `assert`, but cannot be disabled.
// It should be used to detect "external" uncontrolable errors,
// and not for "internal" programmer errors.
//
// See how it's used in ImageLoadPBM, for example.

// Check a condition and if false, print failmsg and exit.
static void check(int condition, const char* failmsg) {
  if (!condition) {
    perror(failmsg);
    exit(errno || 255);
  }
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) {  ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  InstrName[1] = "pixcomp";
  // Name other counters here...
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
#define PIXCOMP InstrCount[1]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Auxiliary (static) functions

static Image AllocateImageHeader(uint32 width, uint32 height) {
  // Create the header of an image data structure
  // Allocate the array of pointers to rows
  // And the look-up table

  Image newHeader = malloc(sizeof(struct image));
  // Error handling
  check(newHeader != NULL, "malloc");

  newHeader->width = width;
  newHeader->height = height;

  // Allocating the array of pointers to image rows
  newHeader->image = malloc(height * sizeof(uint16*));
  // Error handling
  check(newHeader->image != NULL, "Alloc failed ->image array");

  // Allocating the LUT
  newHeader->LUT = malloc(FIXED_LUT_SIZE * sizeof(rgb_t));
  // Error handling
  check(newHeader->LUT != NULL, "Alloc failed ->LUT array");

  // Initialize LUT with 2 fixed colors
  newHeader->LUT[0] = 0xffffff;  // RGB WHITE
  newHeader->LUT[1] = 0x000000;  // RGB BLACK

  newHeader->num_colors = 2;  // FALTAVA ESTA LINHA

  return newHeader;
}

// Allocate row of background (label=0) pixels
static uint16* AllocateRowArray(uint32 size) {
  uint16* newArray = calloc((size_t)size, sizeof(uint16));
  // Error handling
  check(newArray != NULL, "AllocateRowArray");

  return newArray;
}

/// Find color label for given RGB color in img LUT.
/// Return the label or -1 if not found.
static int LUTFindColor(Image img, rgb_t color) {
  for (uint16 index = 0; index < img->num_colors; index++) {
    if (img->LUT[index] == color) return index;
  }
  return -1;
}

/// Return color label for RGB color in img LUT.
/// Finds existing color or allocs new one!
static int LUTAllocColor(Image img, rgb_t color) {
  int index = LUTFindColor(img, color);
  if (index < 0) {
    check(img->num_colors < FIXED_LUT_SIZE, "LUT Overflow");
    index = img->num_colors++;
    img->LUT[index] = color;
  }
  return index;
}

/// Return a pseudo-random successor of the given color.
static rgb_t GenerateNextColor(rgb_t color) {
  return (color + 7639) & 0xffffff;
}

/// Image management functions

/// Create a new RGB image. All pixels with the background WHITE color.
///   width, height: the dimensions of the new image.
/// Requires: width and height must be non-negative.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreate(uint32 width, uint32 height) {
  assert(width > 0);
  assert(height > 0);

  // Just two possible pixel colors
  Image img = AllocateImageHeader(width, height);

  // Creating the image rows
  for (uint32 i = 0; i < height; i++) {
    img->image[i] = AllocateRowArray(width);  // Alloc all WHITE row
  }

  return img;
}

/// Create a new RGB image, with a color chess pattern.
/// The background is WHITE.
///   width, height: the dimensions of the new image.
///   edge: the width and height of a chess square.

/// Requires: width, height and edge must be non-negative.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreateChess(uint32 width, uint32 height, uint32 edge, rgb_t color) {
  assert(width > 0);
  assert(height > 0);
  assert(edge > 0);

  Image img = ImageCreate(width, height);

  // Alloc color in LUT.
  uint16 label = LUTAllocColor(img, color);

  // Assigning the color to each image pixel

  // Pixel (0, 0) gets the chosen color label
  for (uint32 i = 0; i < height; i++) {
    uint32 I = i / edge;
    for (uint32 j = 0; j < width; j++) {
      uint32 J = j / edge;
      img->image[i][j] = (I + J) % 2 ? 0 : label;
    }
  }

  // Return the created chess image
  return img;
}

/// Create an image with a palete of generated colors.
Image ImageCreatePalete(uint32 width, uint32 height, uint32 edge) {
  assert(width > 0);
  assert(height > 0);
  assert(edge > 0);

  Image img = ImageCreate(width, height);

  // Fill LUT with generated colors
  rgb_t color = 0x000000;
  while (img->num_colors < FIXED_LUT_SIZE) {
    color = GenerateNextColor(color);
    img->LUT[img->num_colors++] = color;
  }

  // number of tiles
  uint32 wtiles = width / edge;

  // Pixel (0, 0) gets the chosen color label
  for (uint32 i = 0; i < height; i++) {
    uint32 I = i / edge;
    for (uint32 j = 0; j < width; j++) {
      uint32 J = j / edge;
      img->image[i][j] = (I * wtiles + J) % FIXED_LUT_SIZE;
    }
  }

  return img;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
///
/// Ensures: (*imgp)==NULL.
void ImageDestroy(Image* imgp) {
  assert(imgp != NULL);

  Image img = *imgp;

  for (uint32 i = 0; i < img->height; i++) {
    free(img->image[i]);
  }
  free(img->image);
  free(img->LUT);
  free(img);

  *imgp = NULL;
}

/// Create a deep copy of the image pointed to by img.
///   img : address of an Image variable.
///
/// On success, a new copied image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCopy(const Image img) {   // foi testado ta certo
  assert(img != NULL);
  
  Image newImg = AllocateImageHeader(img->width, img->height);   //Aloca memória para o cabeçalho da Imagem a qual onde ficará a cópia
  
  newImg->num_colors = img->num_colors;                          //Copia a quantidade de cores utilizadas pela imagem original pra copiada

  for(uint16 i = 0; i < img->num_colors; i++){                   //Loop para copiar a LUT da imagem original para a copiada (Utilização de num_colors para percorrer apenas a quantidade 
                                                                 //de cores utilizada.
    newImg->LUT[i] = img->LUT[i];
  }

  for(uint32 i = 0; i < img->height; i++){                       //Loop externo passa por cada elemento da coluna inicial e pra cada elemento aloca espaço para a linha associada aquela                                                                   //posição
    newImg->image[i] = AllocateRowArray(img->width);

    for(uint32 j = 0; j < img->width; j++){                      //Para cada elemento da linha copiamos da imagem original para a cópia
        newImg->image[i][j] = img->image[i][j];
        PIXMEM++;
    }
  }

  return newImg;
}

/// Printing on the console

/// These functions do not modify the image and never fail.

/// Output the raw RGB image (i.e., print the integer value of pixel).
void ImageRAWPrint(const Image img) {
  printf("width = %d height = %d\n", (int)img->width, (int)img->height);
  printf("num_colors = %d\n", (int)img->num_colors);
  printf("RAW image\n");

  // Print the pixel labels of each image row
  for (uint32 i = 0; i < img->height; i++) {
    for (uint32 j = 0; j < img->width; j++) {
      printf("%2d", img->image[i][j]);
    }
    // At current row end
    printf("\n");
  }

  printf("LUT:\n");
  // Print the LUT (R,G,B) values
  for (int i = 0; i < (int)img->num_colors; i++) {
    rgb_t color = img->LUT[i];
    int r = color >> 16 & 0xff;
    int g = color >> 8 & 0xff;
    int b = color & 0xff;
    printf("%3d -> (%3d,%3d,%3d)\n", i, r, g, b);
  }

  printf("\n");
}

/// PBM file operations --- For BW images

// See PBM format specification: http://netpbm.sourceforge.net/doc/pbm.html

//
static void unpackBits(int nbytes, const uint8 bytes[], uint8 raw_row[]) {
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8) {  // or (mask > 0)
    for (int b = 0; b < nbytes; b++) {
      raw_row[8 * b + offset] = (bytes[b] & mask) != 0;
    }
    mask >>= 1;
    offset++;
  }
}

static void packBits(int nbytes, uint8 bytes[], const uint8 raw_row[]) {
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8) {  // or (mask > 0)
    for (int b = 0; b < nbytes; b++) {
      if (offset == 0) bytes[b] = 0;
      bytes[b] |= raw_row[8 * b + offset] ? mask : 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PBM file.
/// Only binary PBM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageLoadPBM(const char* filename) {  ///
  int w, h;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  check((f = fopen(filename, "rb")) != NULL, "Open failed");
  // Parse PBM header
  check(fscanf(f, "P%c ", &c) == 1 && c == '4', "Invalid file format");
  skipComments(f);
  check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width");
  skipComments(f);
  check(fscanf(f, "%d", &h) == 1 && h >= 0, "Invalid height");
  check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected");

  // Allocate image
  img = AllocateImageHeader((uint32)w, (uint32)h);

  // Read pixels
  int nbytes = (w + 8 - 1) / 8;  // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  uint8 raw_row[nbytes * 8];
  for (uint32 i = 0; i < img->height; i++) {
    check(fread(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
          "Reading pixels");
    unpackBits(nbytes, bytes, raw_row);
    img->image[i] = AllocateRowArray((uint32)w);
    for (uint32 j = 0; j < (uint32)w; j++) {
      img->image[i][j] = (uint16)raw_row[j];
    }
  }

  fclose(f);
  return img;
}

/// Save image to PBM file.
/// On success, returns nonzero.
/// On failurie, a partial and invalid file may be left in the system.
int ImageSavePBM(const Image img, const char* filename) {  ///
  assert(img != NULL);
  //assert(img->num_colors == 2);

  int w = (int)img->width;
  int h = (int)img->height;
  FILE* f = NULL;

  check((f = fopen(filename, "wb")) != NULL, "Open failed");
  check(fprintf(f, "P4\n%d %d\n", w, h) > 0, "Writing header failed");

  // Write pixels
  int nbytes = (w + 8 - 1) / 8;  // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  uint8 raw_row[nbytes * 8];
  for (uint32 i = 0; i < img->height; i++) {
    for (uint32 j = 0; j < img->width; j++) {
      raw_row[j] = (uint8)img->image[i][j];
    }
    // Fill padding pixels with WHITE
    memset(raw_row + w, WHITE, nbytes * 8 - w);
    packBits(nbytes, bytes, raw_row);
    check(fwrite(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
          "Writing pixels failed");
  }

  // Cleanup
  fclose(f);

  return 0;
}

/// PPM file operations --- For RGB images
/// Load a raw PPM file.
/// Only ASCII PPM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageLoadPPM(const char* filename) {
  assert(filename != NULL);
  int w, h;
  int levels;
  char c;
  FILE* f = NULL;

  check((f = fopen(filename, "rb")) != NULL, "Open failed");
  // Parse PPM header
  check(fscanf(f, "P%c ", &c) == 1 && c == '3', "Invalid file format");
  skipComments(f);
  check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width");
  skipComments(f);
  check(fscanf(f, "%d", &h) == 1 && h >= 0, "Invalid height");
  skipComments(f);
  check(fscanf(f, "%d", &levels) == 1 && 0 <= levels && levels <= 255,
        "Invalid depth");
  check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected");

  // Allocate image
  Image img = ImageCreate((uint32)w, (uint32)h);

  // Read pixels
  for (uint32 i = 0; i < img->height; i++) {
    for (uint32 j = 0; j < img->width; j++) {
      int r, g, b;
      check(fscanf(f, "%d %d %d", &r, &g, &b) == 3 && 0 <= r && r <= levels &&
                0 <= g && g <= levels && 0 <= b && b <= levels,
            "Invalid pixel color");
      rgb_t color = r << 16 | g << 8 | b;
      uint16 index = LUTAllocColor(img, color);
      img->image[i][j] = index;
      // printf("[%u][%u]: (%d,%d,%d) -> %u (%6x)\n", i, j, r,g,b, index,
      // color);
    }
    //printf(f, "\n");
  }

  fclose(f);
  return img;
}

/// Save image to PPM file.
/// On success, returns nonzero.
/// On failure, a partial and invalid file may be left in the system.
int ImageSavePPM(const Image img, const char* filename) {
  assert(img != NULL);

  int w = (int)img->width;
  int h = (int)img->height;
  FILE* f = NULL;

  check((f = fopen(filename, "wb")) != NULL, "Open failed");
  check(fprintf(f, "P3\n%d %d\n255\n", w, h) > 0, "Writing header failed");

  // The pixel RGB values
  for (uint32 i = 0; i < img->height; i++) {
    for (uint32 j = 0; j < img->width; j++) {
      uint16 index = img->image[i][j];
      rgb_t color = img->LUT[index];
      int r = color >> 16 & 0xff;
      int g = color >> 8 & 0xff;
      int b = color & 0xff;
      fprintf(f, "  %3d %3d %3d", r, g, b);
    }
    fprintf(f, "\n");
  }

  // Cleanup
  fclose(f);

  return 0;
}

/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
uint32 ImageWidth(const Image img) {
  assert(img != NULL);
  return img->width;
}

/// Get image height
uint32 ImageHeight(const Image img) {
  assert(img != NULL);
  return img->height;
}

/// Get number of image colors
uint16 ImageColors(const Image img) {
  assert(img != NULL);
  return img->num_colors;
}

/// Image comparison

/// These functions do not modify the images and never fail.

/// Check if img1 and img2 represent equal images.
/// NOTE: The same rgb color may correspond to different LUT labels in
/// different images!
int ImageIsEqual(const Image img1, const Image img2) {  // foi testado ta certo
  assert(img1 != NULL);
  assert(img2 != NULL);

  PIXCOMP += 2; // +2 comparaçoes no pior caso 
  if(img1->height != img2->height || img1->width != img2->width){  // COMPARAR altura e largura
      return 0;
  }

  for (uint32 i=0;i<img1->height;i++){   // unit32  porque o height foi deifinido com 32 bits 
      for(uint32 j=0;j<img1->width;j++){    // mesmo que o height
          uint16 label_img1 = img1->image[i][j];    //Coloco um label da img1 
          uint16 label_img2 = img2->image[i][j];    //Coloco um label da img2 
          PIXMEM += 2;                                   // Aumentar o N de acessos aos pixeis
         
          PIXCOMP++;
          if(img1->LUT[label_img1] != img2->LUT[label_img2]) return 0; // Comparar os cores 0x______
      }
  }


  return 1;    
}

int ImageIsDifferent(const Image img1, const Image img2) {
  assert(img1 != NULL);
  assert(img2 != NULL);

  return !ImageIsEqual(img1, img2);
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

/// Rotate 90 degrees clockwise (CW).
/// Returns a rotated version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageRotate90CW(const Image img) {    // testado funciona ta certo
  assert(img != NULL);
                                                                                // Aloca memória para os dados da imagem rotacionada, invertendo os valores 
  Image newImg = AllocateImageHeader(img->height, img->width);                  // dos campos width e height para a rotação.
  
  newImg->num_colors = img->num_colors;                                         // Copia num_colors.
  
  for(uint16 k = 0; k < img->num_colors; k++){
    newImg->LUT[k] = img->LUT[k];
  }
  
  for(uint32 u = 0; u < newImg->height; u++){                                   // Aloca espaço para as linhas para cada ponteiro no array image.
    newImg->image[u] = AllocateRowArray(newImg->width);
  }
  
  for(uint32 i = 0; i < img->height; i++){                                      // Copiar pixels aplicando a transformação de rotação 90° CW
    for(uint32 j = 0; j < img->width; j++){
      newImg->image[j][img->height - 1 - i] = img->image[i][j];                 // pixel(i,j) -> (j, new_height - 1 - i)
      PIXMEM++;
    }
  }
  
  return newImg;
}



/// Rotate 180 degrees clockwise (CW).
/// Returns a rotated version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageRotate180CW(const Image img) {  // testado e revisado, ta certo
  assert(img != NULL);
  
  Image newImg = AllocateImageHeader(img->width, img->height);           // alloca o header para criar uma imagem brnca
  
  newImg->num_colors = img->num_colors;                                  // Copia num_colors
  
  for(uint16 k = 0; k < img->num_colors; k++){
    newImg->LUT[k] = img->LUT[k];                                        //copia cada cor da lut antiga para a lut nova
  }
    for(uint32 u = 0; u < newImg->height; u++){                            // Aloca espaço para as linhas para cada ponteiro no array image.
    newImg->image[u] = AllocateRowArray(newImg->width);
  }

  for(uint32 i=0; i<img->height;i++){
      for(uint32 j=0;j<img->width;j++){
          newImg->image[(img->height)-i-1][(img->width)-1-j] = img->image[i][j];        // Executa a rotacao de 180cw
          PIXMEM++;                                                                  // +1 PIXMEM
      }                                                                                 
  }

  return newImg;
}


/// Check whether pixel coords (u, v) are inside img.
/// ATTENTION
///   u : column index
///   v : row index
int ImageIsValidPixel(const Image img, int u, int v) {
  return 0 <= u && u < (int)img->width && 0 <= v && v < (int)img->height;
}

/// Region Growing

/// The following three *RegionFilling* functions perform region growing
/// using some variation of the 4-neighbors flood-filling algorithm:
///   Given the coordinates (u, v) of a seed pixel,
///   fill all similarly-colored adjacent pixels with a new color label.
///
/// All of these functions receive the same arguments:
///   img: The image to operate on (and modify).
///   u, v: the coordinates of the seed pixel.
///   label: the new color label (LUT index) to fill the region with.
///
/// And return: the number of labeled pixels.

/// Each function carries out a different version of the algorithm.

/// Region growing using the recursive flood-filling algorithm.
int ImageRegionFillingRecursive(Image img, int u, int v, uint16 label) { // REVISED, ta certo
  assert(img != NULL);
  assert(ImageIsValidPixel(img, u, v));
  assert(label < FIXED_LUT_SIZE);
    // U width  V height
  int count = 1;
  uint16 current_color = img->image[v][u];         // GUARDA O LABEL DA COR ATUAL 
  PIXMEM++;
  if (current_color == label){ return 0;}              // Se a cor do pixe autal e' igual a label nao faz nada

  img->image[v][u] = label;                      //Muda a cor do pixel atual para o cor desejado, para atraves do index. 

  if(ImageIsValidPixel(img, u+1, v) && img->image[v][u+1] == current_color){             //Testa se o pixel u+1,v tem a mesma cor do original, se sim vai na recursiva
      count += ImageRegionFillingRecursive(img,u+1,v,label);  
  }

  if(ImageIsValidPixel(img, u-1, v) && img->image[v][u-1] == current_color){        //Testa se o pixel u-1,v tem a mesma cor do original, se sim vai na recursiva
      count += ImageRegionFillingRecursive(img,u-1,v,label);  
  }

  if(ImageIsValidPixel(img, u, v+1) && img->image[v+1][u] == current_color){         //Testa se o pixel u,v+1 tem a mesma cor do original, se sim vai na recursiva
     count += ImageRegionFillingRecursive(img,u,v+1,label);  
  }

  if(ImageIsValidPixel(img, u, v-1) && img->image[v-1][u] == current_color){         //Testa se o pixel u,v-1 tem a mesma cor do original, se sim vai na recursiva
     count += ImageRegionFillingRecursive(img,u,v-1,label);  
  }

  return count;
}

/// Region growing using a STACK of pixel coordinates to
/// implement the flood-filling algorithm.
int ImageRegionFillingWithSTACK(Image img, int u, int v, uint16 label) {  // REVISADO e testado
  assert(img != NULL);
  assert(ImageIsValidPixel(img, u, v));
  assert(label < FIXED_LUT_SIZE);

  uint16 originalColor = img->image[v][u];              // Guardamos a cor do pixel inicial.
  PIXMEM++;  
 
  int count_pixel = 0;                                  // Conta quantos pixels foram pintados.

  Stack* stack = StackCreate(img->width * img->height); // Criar ponteiro para stack com o tamanho do array 2d (n * m\).
  
  PixelCoords startPixel = PixelCoordsCreate(u, v);    
  StackPush(stack, startPixel);                         // Coloca as coordenadas do primeiro pixel na pilha/stack.

  if (originalColor == label){ 
      StackDestroy(&stack);
      return 0;
  }    // Se a cor do label for igual a original n faz nada
 
  img->image[v][u] = label;
  PIXMEM++;
  count_pixel++;

  while(!StackIsEmpty(stack)){     
      
    PixelCoords current = StackPop(stack); 
    int x = PixelCoordsGetU(current);
    int y = PixelCoordsGetV(current);                   // Desempilha-mos o próximo pixel.
    
    if(ImageIsValidPixel(img,x,y+1) && img->image[y+1][x]==originalColor){        // Verifica se o pixel é válido ou seja se está dentro da imagem.
    PIXMEM++;                                           
    img->image[y+1][x] = label;                        
    count_pixel++;                                      
    PixelCoords up = PixelCoordsCreate(x, y+1);    
    StackPush(stack, up);    
    }

    if(ImageIsValidPixel(img,x,y-1) && img->image[y-1][x]==originalColor){        // Verifica se o pixel é válido ou seja se está dentro da imagem.
    PIXMEM++;                                           
    img->image[y-1][x] = label;                        
    count_pixel++;                                      
    PixelCoords down = PixelCoordsCreate(x, y-1);    
    StackPush(stack, down);    
    }

    if(ImageIsValidPixel(img,x+1,y) && img->image[y][x+1]==originalColor){        // Verifica se o pixel é válido ou seja se está dentro da imagem.
    PIXMEM++;                                           
    img->image[y][x+1] = label;                        
    count_pixel++;                                      
    PixelCoords right = PixelCoordsCreate(x+1, y);    
    StackPush(stack, right);    
    }

    if(ImageIsValidPixel(img,x-1,y) && img->image[y][x-1]==originalColor){        // Verifica se o pixel é válido ou seja se está dentro da imagem.
    PIXMEM++;                                           
    img->image[y][x-1] = label;                        
    count_pixel++;                                      
    PixelCoords left = PixelCoordsCreate(x-1, y);    
    StackPush(stack, left);    
    }
  }

  StackDestroy(&stack);
  return count_pixel;  
}


/// Region growing using a QUEUE of pixel coordinates to
/// implement the flood-filling algorithm.
int ImageRegionFillingWithQUEUE(Image img, int u, int v, uint16 label) {
  assert(img != NULL);
  assert(ImageIsValidPixel(img, u, v));
  assert(label < FIXED_LUT_SIZE);
 
  int count_pixel = 0;
  Queue* ptr = QueueCreate(img->width * img->height);  // CRIA UM PONTEIRO PARA QUEUE
  uint16 cor_original = img->image[v][u];       // COR ORIGINAL GUARDADA
  PIXMEM++;
  if (cor_original == label){ 
      QueueDestroy(&ptr);
      return 0;
  }    // Se a cor do label for igual a original n faz nada
 
  img->image[v][u] = label;
  PIXMEM++;
  count_pixel++;
  PixelCoords start_pixel = PixelCoordsCreate(u,v);           // CRIA ESTRUTURA DE PIXEL 
  QueueEnqueue(ptr,start_pixel);                      // Coloca o pixel atual na queue
 
  while(!QueueIsEmpty(ptr)){                            // Condicao de para terminar  codigo

  PixelCoords current_pixel = QueueDequeue(ptr);           // pega o pixel atual
  int x = PixelCoordsGetU(current_pixel);                  // coordenadas de U  
  int y = PixelCoordsGetV(current_pixel);                  // coordenadas de V
  
  if(ImageIsValidPixel(img,x,y+1) && img->image[y+1][x]==cor_original){   // Valida as condicoes para logo por na queue o pixel 
      img->image[y+1][x] = label;         // troca a correr 
      count_pixel++;                      // count++
      PIXMEM++;                      
      PixelCoords up = PixelCoordsCreate(x,y+1);   // Cria o pixel
      QueueEnqueue(ptr,up);                        // Adiciona o pixel a queue
  }

  if(ImageIsValidPixel(img,x,y-1) && img->image[y-1][x]==cor_original){
      img->image[y-1][x] = label;
      count_pixel++;
      PIXMEM++;
      PixelCoords down = PixelCoordsCreate(x,y-1);
      QueueEnqueue(ptr,down);
  }

  if(ImageIsValidPixel(img,x+1,y) && img->image[y][x+1]==cor_original){
      img->image[y][x+1] = label;
      count_pixel++;
      PIXMEM++;
      PixelCoords rigth = PixelCoordsCreate(x+1,y);
      QueueEnqueue(ptr,rigth);
  }

  if(ImageIsValidPixel(img,x-1,y) && img->image[y][x-1]==cor_original){
      img->image[y][x-1] = label;
      count_pixel++;
      PIXMEM++;
      PixelCoords left = PixelCoordsCreate(x-1,y);
      QueueEnqueue(ptr,left);
  } 
} 

QueueDestroy(&ptr); 
return count_pixel; }
/// Image Segmentation
/// Label each WHITE region with a different color. 
/// - WHITE (the background color) has label (LUT index) 0. 
/// - Use GenerateNextColor to create the RGB color for each new region. 
/// /// One of the region filling functions above is passed as the
/// last argument, using a function pointer. 
/// /// Returns the number of image regions found.  
int ImageSegmentation(Image img, FillingFunction fillFunct) {  // TESTADA funciona 
    assert(img != NULL); 
    assert(fillFunct != NULL); 
    int numRegions = 0; 
    rgb_t currentColor = 0x000000;           //Inicializa com a cor preta.
    for (uint32 v = 0; v < img->height; v++) {
        for (uint32 u = 0; u < img->width; u++) { 
        // Adicionar PIXMEM????  
            if (img->image[v][u] == 0) {                          // Verifica região branca. 
                currentColor = GenerateNextColor(currentColor);    // Gera um novo cor pseudo aleatoio
                uint16 newLabel = LUTAllocColor(img, currentColor); //Adiciona a cor a LUT da imagem. 
                fillFunct(img, u, v, newLabel);                     //Preenche a area ao redor.
                numRegions++;
            }
        }
    }
  return numRegions;
}
