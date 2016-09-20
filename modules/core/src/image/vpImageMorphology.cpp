/****************************************************************************
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2015 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Image morphology.
 *
 * Authors:
 * Souriya Trinh
 *
 *****************************************************************************/

#include <visp3/core/vpImageMorphology.h>

#if defined __SSE2__ || defined _M_X64 || (defined _M_IX86_FP && _M_IX86_FP >= 2)
#  include <emmintrin.h>
#  define VISP_HAVE_SSE2 1
#endif


/*!
  Erode a grayscale image using the given structuring element.

  The gray-scale erosion of \f$ A \left( x, y \right) \f$ by \f$ B \left (x, y \right) \f$ is defined as:
  \f[
    \left ( A \ominus B \right ) \left( x,y \right) = \textbf{min} \left \{ A \left ( x+x', y+y' \right ) -
    B \left ( x', y'\right ) | \left ( x', y'\right ) \subseteq D_B \right \}
  \f]
  where \f$ D_B \f$ is the domain of the structuring element \f$ B \f$ and \f$ A \left( x,y \right) \f$ is assumed
  to be \f$ + \infty \f$ outside the domain of the image.

  In our case, gray-scale erosion is performed with a flat structuring element \f$ \left( B \left( x,y \right) = 0 \right) \f$.
  Gray-scale erosion using such a structuring element is equivalent to a local-minimum operator:
  \f[
    \left ( A \ominus B \right ) \left( x,y \right) = \textbf{min} \left \{ A \left ( x+x', y+y' \right ) | \left ( x', y'\right ) \subseteq D_B \right \}
  \f]

  \param I : Image to process.
  \param connexity : Type of connexity: 4 or 8.

  \sa dilatation2()
*/
void vpImageMorphology::erosion2(vpImage<unsigned char> &I, vpConnexityType connexity) {
  if(I.getSize() == 0) {
    std::cerr << "Input image is empty!" << std::endl;
    return;
  }

  const unsigned char null_value = 255;

  vpImage<unsigned char> J(I.getHeight()+2, I.getWidth()+2);
  // Copy I to J and add border
  for (unsigned int i = 0; i < J.getHeight(); i++) {
    if (i == 0 || i == J.getHeight() - 1) {
      for (unsigned int j = 0; j < J.getWidth(); j++) {
        J[i][j] = null_value;
      }
    } else {
      for (unsigned int j = 0; j < J.getWidth(); j++) {
        if (j == 0 || j == J.getWidth() - 1) {
          J[i][j] = null_value;
        } else {
          J[i][j] = I[i-1][j-1];
        }
      }
    }
  }

  if (connexity == CONNEXITY_4) {
    unsigned int offset[5] = {1, J.getWidth(), J.getWidth()+1, J.getWidth()+2, J.getWidth()*2 + 1};

    for (unsigned int i = 0; i < I.getHeight(); i++) {
      unsigned int j = 0;

#if VISP_HAVE_SSE2
      if (I.getWidth() >= 16) {

        for (; j <= I.getWidth() - 16; j+=16) {
          __m128i m = _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[0]) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[1])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[2])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[3])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[4])) );

          _mm_storeu_si128( (__m128i *) (I.bitmap + i*I.getWidth() + j), m );
        }
      }
#endif

      for (; j < I.getWidth(); j++) {
        unsigned char min_value = null_value;
        for (int k = 0; k < 5; k++) {
          min_value = std::min(min_value, J.bitmap[i*J.getWidth() + j + offset[k]]);
        }

        I.bitmap[i*I.getWidth() + j] = min_value;
      }
    }
  } else {
    //CONNEXITY_8
    unsigned int offset[9] = {0, 1, 2, J.getWidth(), J.getWidth()+1, J.getWidth()+2,
                              J.getWidth()*2, J.getWidth()*2 + 1, J.getWidth()*2 + 2};

    for (unsigned int i = 0; i < I.getHeight(); i++) {
      unsigned int j = 0;

#if VISP_HAVE_SSE2
      if (I.getWidth() >= 16) {

        for (; j <= I.getWidth() - 16; j+=16) {
          __m128i m = _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[0]) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[1])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[2])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[3])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[4])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[5])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[6])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[7])) );
          m = _mm_min_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[8])) );

          _mm_storeu_si128( (__m128i *) (I.bitmap + i*I.getWidth() + j), m );
        }
      }
#endif

      for (; j < I.getWidth(); j++) {
        unsigned char min_value = null_value;
        for (int k = 0; k < 9; k++) {
          min_value = std::min(min_value, J.bitmap[i*J.getWidth() + j + offset[k]]);
        }

        I.bitmap[i*I.getWidth() + j] = min_value;
      }
    }
  }
}

/*!
  Dilate a grayscale image using the given structuring element.

  The gray-scale dilatation of \f$ A \left( x, y \right) \f$ by \f$ B \left (x, y \right) \f$ is defined as:
  \f[
    \left ( A \oplus B \right ) \left( x,y \right) = \textbf{max} \left \{ A \left ( x-x', y-y' \right ) +
    B \left ( x', y'\right ) | \left ( x', y'\right ) \subseteq D_B \right \}
  \f]
  where \f$ D_B \f$ is the domain of the structuring element \f$ B \f$ and \f$ A \left( x,y \right) \f$ is assumed
  to be \f$ - \infty \f$ outside the domain of the image.

  In our case, gray-scale erosion is performed with a flat structuring element \f$ \left( B \left( x,y \right) = 0 \right) \f$.
  Gray-scale erosion using such a structuring element is equivalent to a local-maximum operator:
  \f[
    \left ( A \oplus B \right ) \left( x,y \right) = \textbf{max} \left \{ A \left ( x-x', y-y' \right ) | \left ( x', y'\right ) \subseteq D_B \right \}
  \f]

  \param I : Image to process.
  \param connexity : Type of connexity: 4 or 8.

  \sa erosion2()
*/
void vpImageMorphology::dilatation2(vpImage<unsigned char> &I, vpConnexityType connexity) {
  if(I.getSize() == 0) {
    std::cerr << "Input image is empty!" << std::endl;
    return;
  }

  const unsigned char null_value = 0;

  vpImage<unsigned char> J(I.getHeight()+2, I.getWidth()+2);
  // Copy I to J and add border
  for (unsigned int i = 0; i < J.getHeight(); i++) {
    if (i == 0 || i == J.getHeight() - 1) {
      for (unsigned int j = 0; j < J.getWidth(); j++) {
        J[i][j] = null_value;
      }
    } else {
      for (unsigned int j = 0; j < J.getWidth(); j++) {
        if (j == 0 || j == J.getWidth() - 1) {
          J[i][j] = null_value;
        } else {
          J[i][j] = I[i-1][j-1];
        }
      }
    }
  }

  if (connexity == CONNEXITY_4) {
    unsigned int offset[5] = {1, J.getWidth(), J.getWidth()+1, J.getWidth()+2, J.getWidth()*2 + 1};

    for (unsigned int i = 0; i < I.getHeight(); i++) {
      unsigned int j = 0;

#if VISP_HAVE_SSE2
      if (I.getWidth() >= 16) {

        for (; j <= I.getWidth() - 16; j+=16) {
          __m128i m = _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[0]) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[1])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[2])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[3])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[4])) );

          _mm_storeu_si128( (__m128i *) (I.bitmap + i*I.getWidth() + j), m );
        }
      }
#endif

      for (; j < I.getWidth(); j++) {
        unsigned char max_value = null_value;
        for (int k = 0; k < 5; k++) {
          max_value = std::max(max_value, J.bitmap[i*J.getWidth() + j + offset[k]]);
        }

        I.bitmap[i*I.getWidth() + j] = max_value;
      }
    }
  } else {
    //CONNEXITY_8
    unsigned int offset[9] = {0, 1, 2, J.getWidth(), J.getWidth()+1, J.getWidth()+2,
                              J.getWidth()*2, J.getWidth()*2 + 1, J.getWidth()*2 + 2};

    for (unsigned int i = 0; i < I.getHeight(); i++) {
      unsigned int j = 0;

#if VISP_HAVE_SSE2
      if (I.getWidth() >= 16) {

        for (; j <= I.getWidth() - 16; j+=16) {
          __m128i m = _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[0]) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[1])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[2])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[3])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[4])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[5])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[6])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[7])) );
          m = _mm_max_epu8(m, _mm_loadu_si128( (const __m128i *) (J.bitmap + i*J.getWidth() + j + offset[8])) );

          _mm_storeu_si128( (__m128i *) (I.bitmap + i*I.getWidth() + j), m );
        }
      }
#endif

      for (; j < I.getWidth(); j++) {
        unsigned char max_value = null_value;
        for (int k = 0; k < 9; k++) {
          max_value = std::max(max_value, J.bitmap[i*J.getWidth() + j + offset[k]]);
        }

        I.bitmap[i*I.getWidth() + j] = max_value;
      }
    }
  }
}
