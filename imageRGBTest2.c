#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "error.h"
#include "imageRGB.h"
#include "instrumentation.h"
 
int main(int argc, char* argv[]) {
    ImageInit();
    program_name = argv[0];
   if (argc != 1) {
     error(1, 0, "Usage: imageRGBTest");
   }
 

printf("\n=== ANALISIS FORMAL DE ImageIsEqual ===\n\n");

// Preparación: Crear imágenes para el test
Image img0 = ImageCreateChess(10, 20, 10, 0x0000000);// 200
Image img1 = ImageCreateChess(20, 20, 10, 0x000000); // 400
Image img2 = ImageCreateChess(40, 20, 10, 0x000000); // 800
Image img3 = ImageCreateChess(80, 20, 10, 0x000000); // 1600
Image img4 = ImageCreateChess(160, 20, 10, 0x000000); // 3200
Image img5 = ImageCreateChess(320, 20, 10, 0x000000); // 6400
Image img6 = ImageCreateChess(640, 20, 10, 0x000000); // 1280
Image img7 = ImageCreateChess(1280, 20, 10, 0x000000); //2560
Image img8 = ImageCreateChess(2560, 20, 10, 0x000000); // 5120
Image img9 = ImageCreateChess(5120, 20, 10, 0x000000); // 





// Modificamos imgD para que sea diferente en el primer píxel (Mejor caso 2)
// (Nota: asumiendo que tienes acceso para modificar o usas una función de dibujo)
// Una forma "sucia" si no tienes funciones de dibujo es asumir que difieren rápido.

// TESTES:
// 50 100 200 300 400 500 600 700 800 1000
// 0   1   2   3   4   5   6   7   8   9

// --- TEST 1: PEOR CASO (Imágenes Idénticas) ---
// Reiniciar contadores a 0

InstrReset();
ImageIsEqual(img0, img0);
InstrPrint(10*20); 
InstrReset();
ImageIsEqual(img1, img1);
InstrPrint(20*20);
InstrReset();
ImageIsEqual(img2, img2);
InstrPrint(40*20); 
InstrReset();
ImageIsEqual(img3, img3);
InstrPrint(80*20);
InstrReset();
ImageIsEqual(img4, img4);
InstrPrint(160*20); 
InstrReset();
ImageIsEqual(img5, img5);
InstrPrint(320*20);
InstrReset();
ImageIsEqual(img6, img6);
InstrPrint(640*20); 
InstrReset();
ImageIsEqual(img7, img7);
InstrPrint(1280*20);
InstrReset();
ImageIsEqual(img8, img8);
InstrPrint(2560*20); 
InstrReset();
ImageIsEqual(img9, img9);
InstrPrint(5120*20);




// Limpieza
ImageDestroy(&img0);
ImageDestroy(&img1);
ImageDestroy(&img2);
ImageDestroy(&img3);
ImageDestroy(&img4);
ImageDestroy(&img5);
ImageDestroy(&img6);
ImageDestroy(&img7);
ImageDestroy(&img8);
ImageDestroy(&img9);


}
