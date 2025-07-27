// =======================================================================
//  ARQUIVO: SRC/IMAGE_PROVIDER.H (VERSÃO FINAL E COMPLETA)
// =======================================================================
#ifndef IMAGE_PROVIDER_H_
#define IMAGE_PROVIDER_H_

#include "model_settings.h" // Inclui as definições de tamanho

// O array de dados da imagem agora usa a constante para definir seu tamanho.
// Isso garante que ele sempre terá o tamanho correto se você mudar as dimensões no futuro.
extern float image_data[kMaxImageSize];

// A função que preenche o array 'image_data'.
// (A implementação dela está em image_provider.cpp)
void FillImage();

#endif // IMAGE_PROVIDER_H_


