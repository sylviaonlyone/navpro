#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

char* readAll(const char* fileName, int* bytes)
{
  static const int iBlockSize = 4096;
  int iBlockCount = 0, iBytesRead = 0;
  char* pBfr = 0;

  FILE* fp = fopen(fileName, "rb");
  if (!fp)
    {
      fprintf(stderr, "Could not open %s.\n", fileName);
      return 0;
    }
  
  pBfr = (char*)malloc(iBlockSize);
  *bytes = 0;
  
  for (;;)
    {
      iBytesRead = fread(pBfr + iBlockCount*iBlockSize, 1, iBlockSize, fp);
      *bytes += iBytesRead;
      if (iBytesRead != iBlockSize)
        break;
      
      ++iBlockCount;
      pBfr = (char*)realloc(pBfr, (iBlockCount+1) * iBlockSize);
    }
  
  fclose(fp);
  return pBfr;
}

int scan(char ** const filePos, const char* endPos, const char* search, char** outputPos)
{
  static const char* ppSearchPart[16];
  static int pSearchLen[16];
  
  int i, j, iSearchParts = 0;
  int iMaxLen = endPos - *filePos;
  int iSearchLen = search ? strlen(search) : 0;
  int iPreviousStart = 0;

  for (i=0; i<=iSearchLen && iSearchParts < 16; ++i)
    {
      if (search[i] == '|' || search[i] == 0)
        {
          pSearchLen[iSearchParts] = i - iPreviousStart;
          ppSearchPart[iSearchParts] = search + iPreviousStart;
          iPreviousStart = i + 1;
          ++iSearchParts;
        }
    }
  for (i=0; i<iMaxLen; ++i)
    {
      if (search)
        {
          for (j=0; j<iSearchParts; ++j)
            if (i < iMaxLen - pSearchLen[j] &&
                !strncmp(*filePos, ppSearchPart[j], pSearchLen[j]))
              return j;
        }
      if (outputPos)
        {
          **outputPos = **filePos;
          ++*outputPos;
        }
      ++*filePos;
    }
  return -1;
}

void insert(char** outputPos, const char* msg, int len)
{
  if (len == 0) len = strlen(msg);
  memcpy(*outputPos, msg, len);
  *outputPos += len;
}

#define LICENSE_MARKER " *\n * IMPORTANT LICENSING INFORMATION\n *\n"

void insertLicense(const char* fileName, const char* license, int licenseSize)
{
  static const char* pLicenseMarker = LICENSE_MARKER;
  static const int iMarkerLen = 41;
  int iFileSize = 0;
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char* pFileData = readAll(fileName, &iFileSize), *pOutputBfr, *pFilePos, *pOutputPos;
  const char* pEndPos = pFileData + iFileSize;
  FILE *fp;
  
  if (!pFileData)
    return;
  
  pOutputBfr = (char*)malloc(iFileSize + licenseSize + 20 + iMarkerLen);
  pOutputPos = pOutputBfr;
  pFilePos = pFileData;

  if (scan(&pFilePos, pEndPos, "Copyright (C)", &pOutputPos) == -1)
    goto end;
  insert(&pOutputPos, "Copyright (C) 2003-", 0);
  strftime(pOutputPos, 5, "%Y", tm);
  pFilePos += 13;
  pOutputPos += 4;
  insert(&pOutputPos, " Intopii.", 0);
  if (scan(&pFilePos, pEndPos, "\n", 0) == -1)
    goto end;

  switch (scan(&pFilePos, pEndPos, " */|" LICENSE_MARKER, &pOutputPos))
    {
    case -1: goto end;
    case 0: pFilePos += 3; break;
    case 1:
      // License already exists -> get rid of it
      if (scan(&pFilePos, pEndPos, "*/", 0) == -1)
        goto end;
      pFilePos += 2;
    }

  insert(&pOutputPos, pLicenseMarker, iMarkerLen);

  insert(&pOutputPos, license, licenseSize);
  insert(&pOutputPos, " */", 0);

  fp = fopen(fileName, "wb");
  if (!fp)
    goto end;

  fwrite(pOutputBfr, pOutputPos - pOutputBfr, 1, fp);
  fwrite(pFilePos, pEndPos - pFilePos, 1, fp);
  fclose(fp);
  
 end:;
  
  free(pOutputBfr);
  free(pFileData);
}

int main(int argc, char* argv[])
{
  char *pLicenseData;
  int i = 0, iLicenseSize = 0;
  
  if (argc < 3)
    {
      fprintf(stderr, "Usage: licencechanger licensefile sourcefile ...\n");
      return 1;
    }

  pLicenseData = readAll(argv[1], &iLicenseSize);
  if (!pLicenseData)
    return 2;

  for (i=2; i<argc; ++i)
    {
	  //printf("Installing %s to %s\n", argv[1], argv[i]);
      insertLicense(argv[i], pLicenseData, iLicenseSize);
    }

  free(pLicenseData);
  return 0;
}
