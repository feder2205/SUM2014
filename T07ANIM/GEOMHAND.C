/* FILENAME: GEOMHAND.C
 * PROGRAMMER: EF2
 * PURPOSE: Geometry object handle functions.
 * LAST UPDATE: 14.06.2014
 */

#include <stdlib.h>

#include "anim.h"

/* ������� ���������� ��������� � �������.
 * ���������:
 *   - �������������� ������:
 *       ef2GEOM *G;
 *   - ����������� ��������:
 *       ef2MATERIAL *Mtl;
 * ������������ ��������:
 *   (INT) ����� ������������ ��������� ��� -1
 *         ��� ������.
 */
INT EF2_GeomAddMaterial( ef2GEOM *G, ef2MATERIAL *Mtl )
{
  INT i;
  ef2MATERIAL *M;

  /* ���� �������� � ���������� */
  for (i = 0; i < G->NumOfMtls; i++)
    if (strcmp(G->Mtls[i].Name, Mtl->Name)== 0)
    {
      G->Mtls[i] = *Mtl;
      return i;
    }

  if ((M = malloc(sizeof(ef2MATERIAL) * (G->NumOfMtls + 1))) == NULL)
    return -1;
  if (G->Mtls != NULL)
  {
    /* �������� ������ ������ */
    memcpy(M, G->Mtls, sizeof(ef2MATERIAL) * G->NumOfMtls);
    /* ����������� ������ */
    free(G->Mtls);
  }
  /* ��������� */
  G->Mtls = M;
  M[G->NumOfMtls] = *Mtl;
  return G->NumOfMtls++;
} /* End of 'EF2_GeomAddMaterial' function */

/* ������� ���������� ��������� � �������.
 * ���������:
 *   - �������������� ������:
 *       ef2GEOM *G;
 *   - ����������� ��������:
 *       ef2PRIM *Prim;
 * ������������ ��������:
 *   (INT) ����� ������������ ��������� ��� -1
 *         ��� ������.
 */
INT EF2_GeomAddPrim( ef2GEOM *G, ef2PRIM *Prim )
{
  ef2PRIM *P;

  if ((P = malloc(sizeof(ef2PRIM) * (G->NumOfPrims + 1))) == NULL)
    return -1;
  if (G->Prims != NULL)
  {
    /* �������� ������ ������ */
    memcpy(P, G->Prims, sizeof(ef2PRIM) * G->NumOfPrims);
    /* ����������� ������ */
    free(G->Prims);
  }
  /* ��������� */
  G->Prims = P;
  P[G->NumOfPrims] = *Prim;
  return G->NumOfPrims++;
} /* End of 'EF2_GeomAddPrim' function */

/* ������� ������������ */
VOID EF2_GeomFree( ef2GEOM *G )
{
  INT i;

  for (i = 0; i < G->NumOfPrims; i++)
    EF2_PrimFree(G->Prims + i);
  free(G->Prims);
  for (i = 0; i < G->NumOfMtls; i++)
    glDeleteTextures(1, &G->Mtls[i].TexNo);
  free(G->Mtls);
  memset(G, 0, sizeof(ef2GEOM));
} /* End of 'EF2_GeomFree' function */

/* ������� ��������� */
VOID EF2_GeomDraw( ef2GEOM *G )
{
  INT i, loc;
  MATR WVP, MatrWorldInvTrans;
  VEC V;

  /* ��������� ������� �������������� */
  WVP = EF2_MatrMult4x4(EF2_Anim.MatrWorld,
    EF2_MatrMult4x4(EF2_Anim.MatrView, EF2_Anim.MatrProjection));
  MatrWorldInvTrans = MatrTranspose(MatrInverse(EF2_Anim.MatrWorld));

  /* ����� ��������� �������� ������ ���������� */
  glUseProgram(EF2_ShaderProg);
  loc = glGetUniformLocation(EF2_ShaderProg, "MatrWVP");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, WVP.A[0]);

  loc = glGetUniformLocation(EF2_ShaderProg, "MatrWorldInverseTranspose");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, MatrWorldInvTrans.A[0]);

  loc = glGetUniformLocation(EF2_ShaderProg, "MatrWorld");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, EF2_Anim.MatrWorld.A[0]);

  loc = glGetUniformLocation(EF2_ShaderProg, "MatrView");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, EF2_Anim.MatrView.A[0]);

  loc = glGetUniformLocation(EF2_ShaderProg, "Time");
  if (loc != -1)
    glUniform1f(loc, EF2_Anim.Time);

  V = VecSet(-EF2_Anim.MatrView.A[0][2],
             -EF2_Anim.MatrView.A[1][2],
             -EF2_Anim.MatrView.A[2][2]);
  loc = glGetUniformLocation(EF2_ShaderProg, "ViewDir");
  if (loc != -1)
    glUniform3fv(loc, 1, &V.X);


  for (i = 0; i < G->NumOfPrims; i++)
  {
    INT mtl = G->Prims[i].Mtl;

    /* �������������� �������� */
    if (G->Mtls != NULL)
    {
      INT loc;

      if (G->Mtls[mtl].TexNo == 0 && G->Mtls[mtl].MapD[0] != 0)
      {
        INT j;
        IMAGE Img;
                       
        ImageLoad(&Img, G->Mtls[mtl].MapD);

        /* �������� ��������� ����� �������� */
        glGenTextures(1, &G->Mtls[mtl].TexNo);
        /* ������ �� �������� */
        glBindTexture(GL_TEXTURE_2D, G->Mtls[mtl].TexNo);
        for (j = 0; j < Img.W * Img.H; j++)
          Img.Bits[j] |= 0xFF000000;
        /* ���������� �������� � ����������� */
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, Img.W, Img.H,
          GL_BGRA_EXT, GL_UNSIGNED_BYTE, Img.Bits);

        /* ��������� ������ */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
      }
      /* �������� ��������� */
      if (G->Mtls[mtl].TexNo != 0)
      {
        loc = glGetUniformLocation(EF2_ShaderProg, "DrawTexture");
        if (loc != -1)
          glUniform1i(loc, 0);
        /*glEnable(GL_TEXTURE_2D);*/
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, G->Mtls[mtl].TexNo);
        /*glActiveTexture(GL_TEXTURE1);*/
        //glBindTexture(GL_TEXTURE_2D, G->Mtls[mtl].TexNo);
      }
      loc = glGetUniformLocation(EF2_ShaderProg, "Ka");
      if (loc != -1)
        glUniform3fv(loc, 1, &G->Mtls[mtl].Ka.X);
      loc = glGetUniformLocation(EF2_ShaderProg, "Kd");
      if (loc != -1)
        glUniform3fv(loc, 1, &G->Mtls[mtl].Kd.X);
      loc = glGetUniformLocation(EF2_ShaderProg, "Ks");
      if (loc != -1)
        glUniform3fv(loc, 1, &G->Mtls[mtl].Ks.X);
      loc = glGetUniformLocation(EF2_ShaderProg, "Phong");
      if (loc != -1)
        glUniform1f(loc, G->Mtls[mtl].Phong);
      loc = glGetUniformLocation(EF2_ShaderProg, "Trans");
      if (loc != -1)
        glUniform1f(loc, G->Mtls[mtl].Trans);
    }
    EF2_PrimDraw(G->Prims + i);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
} /* End of 'EF2_GeomDraw' function */

/* END OF 'GEOMHAND.C' FILE */
