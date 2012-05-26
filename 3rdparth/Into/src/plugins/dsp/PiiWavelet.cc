/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "PiiWavelet.h"

namespace PiiDsp
{
  PiiMatrix<double> daubechiesScalingFilter(int index)
  {
    static double db1[] = {
			0.50000000000000, 0.50000000000000
		};

    static double db2[] = {
      0.34150635094622, 0.59150635094587, 0.15849364905378, -0.09150635094587
    };

    static double db3[] = {
      0.23523360389270, 0.57055845791731, 0.32518250026371, -0.09546720778426,
      -0.06041610415535, 0.02490874986589
    };

    static double db4[] = {
      0.16290171402562, 0.50547285754565, 0.44610006912319, -0.01978751311791,
      -0.13225358368437, 0.02180815023739, 0.02325180053556, -0.00749349466513
    };

    static double db5[] = {
      0.11320949129173, 0.42697177135271, 0.51216347213016, 0.09788348067375,
      -0.17132835769133, -0.02280056594205, 0.05485132932108, -0.00441340005433,
      -0.00889593505093, 0.00235871396920
    };

    static double db6[] = {
      0.07887121600143, 0.34975190703757, 0.53113187994121, 0.22291566146505,
      -0.15999329944587, -0.09175903203003, 0.06894404648720, 0.01946160485396,
      -0.02233187416548, 0.00039162557603, 0.00337803118151, -0.00076176690258
    };
	
    static double db7[] = {
      0.05504971537285, 0.28039564181304, 0.51557424581833, 0.33218624110566,
      -0.10175691123173, -0.15841750564054, 0.05042323250485, 0.05700172257986,
      -0.02689122629486, -0.01171997078235, 0.00887489618962, 0.00030375749776,
      -0.00127395235906, 0.00025011342658
    };
	
    static double db8[] = {
      0.03847781105406, 0.22123362357624, 0.47774307521438, 0.41390826621166,
      -0.01119286766665, -0.20082931639111, 0.00033409704628, 0.09103817842345,
      -0.01228195052300, -0.03117510332533, 0.00988607964808, 0.00618442240954,
      -0.00344385962813, -0.00027700227421, 0.00047761485533, -0.00008306863060
    };
	
    static double db9[] = {
      0.02692517479416, 0.17241715192471, 0.42767453217028, 0.46477285717278,
      0.09418477475112, -0.20737588089628, -0.06847677451090, 0.10503417113714,
      0.02172633772990, -0.04782363205882, 0.00017744640673, 0.01581208292614,
      -0.00333981011324, -0.00302748028715, 0.00130648364018, 0.00016290733601,
      -0.00017816487955, 0.00002782275679
    };
	
    static double db10[] = {
      0.01885857879640, 0.13306109139687, 0.37278753574266, 0.48681405536610,
      0.19881887088440, -0.17666810089647, -0.13855493935993, 0.09006372426666,
      0.06580149355070, -0.05048328559801, -0.02082962404385, 0.02348490704841,
      0.00255021848393, -0.00758950116768, 0.00098666268244, 0.00140884329496,
      -0.00048497391996, -0.00008235450295, 0.00006617718320, -0.00000937920789
    };

    static double *filters[] = { db1, db2, db3, db4, db5, db6, db7, db8, db9, db10 };

    return PiiMatrix<double>(1, index<<1, filters[index-1]);
  }

  PiiMatrix<double> scalingFilter(WaveletFamily family, int index)
  {
    switch (family)
      {
      case Daubechies:
        if (index < 1 || index > 10)
          index = 2;
        return daubechiesScalingFilter(index);
      case Haar:
      default:
        return daubechiesScalingFilter(1);
      }
  }
}
