void fast9_make_offsets(int pixel[], int row_stride)
{
  pixel[0] = 0 + row_stride * 3;
  pixel[1] = 1 + row_stride * 3;
  pixel[2] = 2 + row_stride * 2;
  pixel[3] = 3 + row_stride * 1;
  pixel[4] = 3 + row_stride * 0;
  pixel[5] = 3 + row_stride * -1;
  pixel[6] = 2 + row_stride * -2;
  pixel[7] = 1 + row_stride * -3;
  pixel[8] = 0 + row_stride * -3;
  pixel[9] = -1 + row_stride * -3;
  pixel[10] = -2 + row_stride * -2;
  pixel[11] = -3 + row_stride * -1;
  pixel[12] = -3 + row_stride * 0;
  pixel[13] = -3 + row_stride * 1;
  pixel[14] = -2 + row_stride * 2;
  pixel[15] = -1 + row_stride * 3;
}
