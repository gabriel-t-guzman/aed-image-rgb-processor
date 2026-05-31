// imageRGBTest - A program that performs some operations on RGB images.
//
// This program is an example use of the imageRGB module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
// 2025

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "imageRGB.h"
#include "instrumentation.h"

int main(int argc, char* argv[]) {
  program_name = argv[0];
  if (argc != 1) {
    error(1, 0, "Usage: imageRGBTest");
  }

  ImageInit();

  // Creating and displaying some images

  printf("1) ImageCreate\n");
  Image white_image = ImageCreate(10, 10);
  // ImageRAWPrint(white_image);

  printf("2) ImageCreateChess(black)+ ImageSavePBM\n");
  Image image_chess_1 = ImageCreateChess(15, 12, 5, 0x000000);  // black
  // ImageRAWPrint(image_chess_1);
  ImageSavePBM(image_chess_1, "chess_image_1.pbm");

  printf("3) ImageCreateChess(red) + ImageSavePPM\n");
  Image image_chess_2 = ImageCreateChess(20, 20, 5, 0xff0000);  // red
  ImageRAWPrint(image_chess_2);
  ImageSavePPM(image_chess_2, "chess_image_2.ppm");

  printf("4) ImageCreateChess(all black)\n");
  Image black_image = ImageCreateChess(10, 10, 10, 0x000000);  // all black
  // ImageRAWPrint(black_image);
  ImageSavePBM(black_image, "black_image.pbm");

  printf("5) ImageCopy\n");
  Image copy_image = ImageCopy(image_chess_1);
  // ImageRAWPrint(copy_image);
  if (copy_image != NULL) {
    ImageSavePBM(copy_image, "copy_image.pbm");
  }

  printf("6) ImageLoadPBM\n");
  Image image_1 = ImageLoadPBM("img/feep.pbm");
  ImageRAWPrint(image_1);

  printf("7) ImageLoadPPM\n");
  Image image_2 = ImageLoadPPM("img/feep.ppm");
  ImageRAWPrint(image_2);


  printf("----------8) Rotatation 90 degrees----------\n");
  Image image_1_rotated_90 = ImageRotate90CW(image_1);
  ImageRAWPrint(image_1_rotated_90);
  ImageSavePPM(image_1_rotated_90,"rotatedFeep.ppm");

  printf("----------9) Rotatation 180 degrees----------\n");
  Image image_1_rotated_180 = ImageRotate180CW(image_1);
  ImageRAWPrint(image_1_rotated_180);
  ImageSavePPM(image_1_rotated_180,"rotated180Feep.ppm");

  printf("10) ImageCreatePalete\n");
  Image image_3 = ImageCreatePalete(4 * 32, 4 * 32, 4);
  ImageSavePPM(image_3, "palete.ppm");


  printf("------------11) Imagecopy chess and IsEqual------------------\n");
  Image rotated_redchess = ImageRotate180CW(image_chess_2); 
  int a = ImageIsEqual(image_chess_2,rotated_redchess);
  printf("E igual: %d\n",a);

  printf("----------12) RegionFilling recursive test ------------\n");
  
  int b =ImageRegionFillingRecursive(image_chess_2, 7, 4, 1);
  ImageSavePPM(image_chess_2,"RegionFIlligntest");
  printf(" n quadrinos recursiva: %d \n", b);

  printf("----------13) RegionFilling  test ------------\n");
  
  int d =ImageRegionFillingWithSTACK(image_chess_2, 15, 16, 1);
  ImageSavePPM(image_chess_2,"RegionFIllignteStack");
  printf(" n quadrinhos Stack: %d \n", d);


  printf("----------14) RegionFilling recursive test ------------\n");
  
  int c = ImageRegionFillingWithQUEUE(image_chess_2, 6, 12, 1);
  ImageSavePPM(image_chess_2,"RegionFIlligntestQeueue");
  printf(" n quadrinhos Queue: %d \n", c);

  printf("----------15 ImageSegmentation test-------------------\n");

  FillingFunction funcao = ImageRegionFillingRecursive;
  int e = ImageSegmentation(image_chess_2,funcao);
  ImageSavePPM(image_chess_2,"RegionSegmentation Test");
  printf(" n quadrinhos visitados pela segmentation: %d \n", e);

  ImageDestroy(&white_image);
  ImageDestroy(&black_image);
  if (copy_image != NULL) {
    ImageDestroy(&copy_image);
  }
  ImageDestroy(&image_chess_1);
  ImageDestroy(&image_chess_2);
  ImageDestroy(&image_1);
  ImageDestroy(&image_2);
  ImageDestroy(&image_3);
  ImageDestroy(&image_1_rotated_180);
  ImageDestroy(&image_1_rotated_90);
  ImageDestroy(&rotated_redchess);
  //ImageDestroy(&);


  return 0;
}
