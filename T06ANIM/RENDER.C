/* FILENAME: RENDER.C
 * PROGRAMMER: EF2
 * PURPOSE: 3D render handle module.
 * LAST UPDATE: 10.06.2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"

/* ���������� ������ */
ef2CAMERA EF2_RndCam;

/* ��������� ������������� */
DBL
  EF2_RndWs = 30, EF2_RndHs = 30,   /* ������� ����� � ������ */
  EF2_RndWp = 4, EF2_RndHp = 3,     /* ������� ������� ������������� */
  EF2_RndProjDist = 5;              /* ���������� �� ��������� �������� */

/* ������� */
MATR
  EF2_RndMatrWorld =  /* ������� �������������� ������� �� */
  {                        
    {                      
      {1, 0, 0, 0},        
      {0, 1, 0, 0},        
      {0, 0, 1, 0},        
      {0, 0, 0, 1},        
    }                      
  },
  EF2_RndMatrView = 
  {                        
    {                      
      {1, 0, 0, 0},        
      {0, 1, 0, 0},        
      {0, 0, 1, 0},        
      {0, 0, 0, 1},        
    }                      
  },
  EF2_RndMatrProjection = 
  {                        
    {                      
      {1, 0, 0, 0},        
      {0, 1, 0, 0},        
      {0, 0, 1, 0},        
      {0, 0, 0, 1},        
    }                      
  };
static MATR
  EF2_RndMatrWorldViewProj;              /* Final matrix */


/* Get final matrix function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID EF2_RndMatrSetup( VOID )
{
  EF2_RndMatrWorldViewProj =
    EF2_MatrMult4x4(EF2_MatrMult4x4(EF2_RndMatrWorld, EF2_RndMatrView),
      EF2_RndMatrProjection);

} /* End of 'EF2_RndMatrSetup' function */

/* ������� �������������� �� ������� ������� ��������� � ����.
 * ���������:
 *   - �������� �����:
 *       VEC P;
 * ������������ ��������:
 *   (POINT) ���������� � �����.
 */
POINT EF2_RndWorldToScreen( VEC P )
{
  POINT Ps;
  VEC Pp;

  /* �������������� �� */
  Pp = EF2_MatrMultVec(EF2_RndMatrWorldViewProj, P);
  
  /* ������������� */
  Pp.X *= EF2_RndProjDist / Pp.Z;
  Pp.Y *= EF2_RndProjDist / Pp.Z;

  Ps.x = (Pp.X + EF2_RndWp / 2) / EF2_RndWp * (EF2_RndWs - 1);
  Ps.y = (-Pp.Y + EF2_RndHp / 2) / EF2_RndHp * (EF2_RndHs - 1);

  return Ps;
} /* End of 'EF2_RndWorldToScreen' function */

/* ������� �������� ��������������� �������.
 * ���������:
 *   - ��������� �� ��������� ��� ����������� ���������:
 *       ef2GOBJ *GObj;
 *   - ��� ����� (*.OBJ):
 *       CHAR *FileName;
 * ������������ ��������:
 *   (BOOL) TRUE ��� ������.
 */
BOOL EF2_RndGObjLoad( ef2GOBJ *GObj, CHAR *FileName )
{
  INT nv = 0, nf = 0;
  FILE *F;
  static CHAR Buf[1000];

  /* �������� ������ */
  memset(GObj, 0, sizeof(ef2GOBJ));
  if ((F = fopen(FileName, "r")) == NULL)
    return 0;

  /* ������� ���������� ������ � ������ */
  while (fgets(Buf, sizeof(Buf), F) != NULL)
  {
    if (Buf[0] == 'v' && Buf[1] == ' ')
      nv++;
    else if (Buf[0] == 'f' && Buf[1] == ' ')
      nf++;
  }

  /* �������� ������ ��� ������� � ����� ��� ������ ������� ������
   * (memory bulk) */
  GObj->V = malloc(nv * sizeof(VEC) + nf * sizeof(INT [3]));
  GObj->F = (INT (*)[3])(GObj->V + nv);
  GObj->NumOfV = nv;
  GObj->NumOfF = nf;

  /* ��������� ������ */
  nv = 0;
  nf = 0;
  rewind(F);
  while (fgets(Buf, sizeof(Buf), F) != NULL)
  {
    DBL x, y, z;
    INT a, b, c;

    if (Buf[0] == 'v' && Buf[1] == ' ')
    {
      sscanf(Buf + 2, "%lf%lf%lf", &x, &y, &z);
      GObj->V[nv++] = VecSet(x, y, z);
    }
    else if (Buf[0] == 'f' && Buf[1] == ' ')
    {
      if (sscanf(Buf + 2, "%i/%*i/%*i %i/%*i/%*i %i/%*i/%*i", &a, &b, &c) == 3 ||
        sscanf(Buf + 2, "%i//%*i %i//%*i %i//%*i", &a, &b, &c) == 3 ||
        sscanf(Buf + 2, "%i/%*i %i/%*i %i/%*i", &a, &b, &c) == 3 ||
        sscanf(Buf + 2, "%i %i %i", &a, &b, &c))
      {
        GObj->F[nf][0] = a - 1;
        GObj->F[nf][1] = b - 1;
        GObj->F[nf][2] = c - 1;
        nf++;
      }
    }
  }
  fclose(F);
  return TRUE;
} /* End of 'EF2_RndGObjLoad' function */

/* ������� ������������ ��������������� �������.
 * ���������:
 *   - ��������� �� ��������� ��� ����������� ���������:
 *       ef2GOBJ *GObj;
 * ������������ ��������: ���.
 */
VOID EF2_RndGObjFree( ef2GOBJ *GObj )
{
  if (GObj->V != NULL)
    free(GObj->V);
  memset(GObj, 0, sizeof(ef2GOBJ));
} /* End of 'EF2_RndGObjFree' function */

/* ������� ��������� ��������������� �������.
 * ���������:
 *   - ��������� �� ��������� ��� ����������� ���������:
 *       ef2GOBJ *GObj;
 *   - �������� ���������� ������:
 *       HDC hDC;
 * ������������ ��������: ���.
 */
VOID EF2_RndGObjDraw( ef2GOBJ *GObj, HDC hDC )
{
  INT i, s = 1;
  POINT *pts;

  if ((pts = malloc(sizeof(POINT) * GObj->NumOfV)) == NULL)
    return;

  EF2_RndMatrSetup();
  for (i = 0; i < GObj->NumOfV; i++)
    pts[i] = EF2_RndWorldToScreen(GObj->V[i]);
 
  for (i = 0; i < GObj->NumOfF; i++)
  {
    INT
      n0 = GObj->F[i][0],
      n1 = GObj->F[i][1],
      n2 = GObj->F[i][2];
    
      MoveToEx(hDC, pts[n0].x, pts[n0].y, NULL);
      LineTo(hDC, pts[n1].x, pts[n1].y);
      LineTo(hDC, pts[n2].x, pts[n2].y);
      LineTo(hDC, pts[n0].x, pts[n0].y);
  }
  free(pts);
} /* End of 'EF2_RndGObjDraw' function */

/* END OF 'RENDER.C' FILE */
