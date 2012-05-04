/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef M2S_OPENGL_H
#define M2S_OPENGL_H 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <evergreen-emu.h>
#include <GL/gl.h>

#define EVG_SYS_OPENGL_IMPL_VERSION_MAJOR		4
#define EVG_SYS_OPENGL_IMPL_VERSION_MINOR		2
#define EVG_SYS_OPENGL_IMPL_VERSION_BUILD		0
#define EVG_SYS_OPENGL_IMPL_VERSION			((EVG_SYS_OPENGL_IMPL_VERSION_MAJOR << 16) | \
						(EVG_SYS_OPENGL_IMPL_VERSION_MINOR << 8) | \
						EVG_SYS_OPENGL_IMPL_VERSION_BUILD)

#define SYS_CODE_OPENGL  326
#define SYS_OPENGL_FUNC_FIRST  1000

/* FIXME */
#define SYS_OPENGL_FUNC_LAST  
#define SYS_OPENGL_FUNC_COUNT  
#define SYS_OPENGL_MAX_ARGS  


/*
 * Miscellaneous
 */

#define OPENGL_FUNC_glClearIndex		1000
#define OPENGL_FUNC_glClearColor			1001
#define OPENGL_FUNC_glClear			1002
#define OPENGL_FUNC_glIndexMask		1003
#define OPENGL_FUNC_glColorMask			1004
#define OPENGL_FUNC_glAlphaFunc			1005
#define OPENGL_FUNC_glBlendFunc			1006
#define OPENGL_FUNC_glLogicOp			1007
#define OPENGL_FUNC_glCullFace			1008
#define OPENGL_FUNC_glFrontFace			1009
#define OPENGL_FUNC_glPointSize			1010
#define OPENGL_FUNC_glLineWidth			1011
#define OPENGL_FUNC_glLineStipple		1012
#define OPENGL_FUNC_glPolygonMode		1013
#define OPENGL_FUNC_glPolygonOffset		1014
#define OPENGL_FUNC_glPolygonStipple		1015
#define OPENGL_FUNC_glGetPolygonStipple	1016
#define OPENGL_FUNC_glEdgeFlag			1017
#define OPENGL_FUNC_glEdgeFlagv			1018
#define OPENGL_FUNC_glScissor			1019
#define OPENGL_FUNC_glClipPlane			1020
#define OPENGL_FUNC_glGetClipPlane		1021
#define OPENGL_FUNC_glDrawBuffer		1022
#define OPENGL_FUNC_glReadBuffer		1023
#define OPENGL_FUNC_glEnable			1024
#define OPENGL_FUNC_glDisable			1025
#define OPENGL_FUNC_glIsEnabled			1026
#define OPENGL_FUNC_glEnableClientState		1027
#define OPENGL_FUNC_glDisableClientState	1028
#define OPENGL_FUNC_glGetBooleanv		1029
#define OPENGL_FUNC_glGetDoublev		1030
#define OPENGL_FUNC_glGetFloatv			1031
#define OPENGL_FUNC_glGetIntegerv		1032
#define OPENGL_FUNC_glPushAttrib			1033
#define OPENGL_FUNC_glPopAttrib			1034
#define OPENGL_FUNC_glPushClientAttrib		1035
#define OPENGL_FUNC_glPopClientAttrib		1036
#define OPENGL_FUNC_glRenderMode		1037
#define OPENGL_FUNC_glGetError			1038
#define OPENGL_FUNC_glGetString			1039
#define OPENGL_FUNC_glFinish			1040
#define OPENGL_FUNC_glFlush			1041
#define OPENGL_FUNC_glHint			1042


/*
 * Depth Buffer
 */

#define OPENGL_FUNC_glClearDepth		1043
#define OPENGL_FUNC_glDepthFunc		1044
#define OPENGL_FUNC_glDepthMask		1045
#define OPENGL_FUNC_glDepthRange		1046


/*
 * Accumulation Buffer
 */

#define OPENGL_FUNC_glClearAccum		1047
#define OPENGL_FUNC_glAccum			1048


/*
 * Transformation
 */

#define OPENGL_FUNC_glMatrixMode		1049
#define OPENGL_FUNC_glOrtho			1050
#define OPENGL_FUNC_glFrustum			1051
#define OPENGL_FUNC_glViewport			1052
#define OPENGL_FUNC_glPushMatrix		1053
#define OPENGL_FUNC_glPopMatrix			1054
#define OPENGL_FUNC_glLoadIdentity		1055
#define OPENGL_FUNC_glLoadMatrixd		1056
#define OPENGL_FUNC_glLoadMatrixf		1057
#define OPENGL_FUNC_glMultMatrixd		1058
#define OPENGL_FUNC_glMultMatrixf		1059
#define OPENGL_FUNC_glRotated			1060
#define OPENGL_FUNC_glRotatef			1061
#define OPENGL_FUNC_glScaled			1062
#define OPENGL_FUNC_glScalef			1063
#define OPENGL_FUNC_glTranslated			1064
#define OPENGL_FUNC_glTranslatef			1065


 /*
 * Display Lists
 */

#define OPENGL_FUNC_glIsList			1066
#define OPENGL_FUNC_glDeleteLists		1067
#define OPENGL_FUNC_glGenLists			1068
#define OPENGL_FUNC_glNewList			1069
#define OPENGL_FUNC_glEndList			1070
#define OPENGL_FUNC_glCallList			1071
#define OPENGL_FUNC_glCallLists			1072
#define OPENGL_FUNC_glListBase			1073


/*
 * Drawing Functions
 */

#define OPENGL_FUNC_glBegin			1074
#define OPENGL_FUNC_glEnd			1075
#define OPENGL_FUNC_glVertex2d			1076
#define OPENGL_FUNC_glVertex2f			1077
#define OPENGL_FUNC_glVertex2i			1078
#define OPENGL_FUNC_glVertex2s			1079
#define OPENGL_FUNC_glVertex3d			1080
#define OPENGL_FUNC_glVertex3f			1081
#define OPENGL_FUNC_glVertex3i			1082
#define OPENGL_FUNC_glVertex3s			1083
#define OPENGL_FUNC_glVertex4d			1084
#define OPENGL_FUNC_glVertex4f			1085
#define OPENGL_FUNC_glVertex4i			1086
#define OPENGL_FUNC_glVertex4s			1087
#define OPENGL_FUNC_glVertex2dv			1088
#define OPENGL_FUNC_glVertex2fv			1089
#define OPENGL_FUNC_glVertex2iv			1090
#define OPENGL_FUNC_glVertex2sv			1091
#define OPENGL_FUNC_glVertex3dv			1092
#define OPENGL_FUNC_glVertex3fv			1093
#define OPENGL_FUNC_glVertex3iv			1094
#define OPENGL_FUNC_glVertex3sv			1095
#define OPENGL_FUNC_glVertex4dv			1096
#define OPENGL_FUNC_glVertex4fv			1097
#define OPENGL_FUNC_glVertex4iv			1098
#define OPENGL_FUNC_glVertex4sv			1099
#define OPENGL_FUNC_glNormal3b			1100
#define OPENGL_FUNC_glNormal3d			1101
#define OPENGL_FUNC_glNormal3f			1102
#define OPENGL_FUNC_glNormal3i			1103
#define OPENGL_FUNC_glNormal3s			1104
#define OPENGL_FUNC_glNormal3bv		1105
#define OPENGL_FUNC_glNormal3dv		1106
#define OPENGL_FUNC_glNormal3fv			1107
#define OPENGL_FUNC_glNormal3iv			1108
#define OPENGL_FUNC_glNormal3sv			1109
#define OPENGL_FUNC_glIndexd			1110
#define OPENGL_FUNC_glIndexf			1111
#define OPENGL_FUNC_glIndexi			1112
#define OPENGL_FUNC_glIndexs			1113
#define OPENGL_FUNC_glIndexub			1114
#define OPENGL_FUNC_glIndexdv			1115
#define OPENGL_FUNC_glIndexfv			1116
#define OPENGL_FUNC_glIndexiv			1117
#define OPENGL_FUNC_glIndexsv			1118
#define OPENGL_FUNC_glIndexubv			1119
#define OPENGL_FUNC_glColor3b			1120
#define OPENGL_FUNC_glColor3d			1121
#define OPENGL_FUNC_glColor3f			1122
#define OPENGL_FUNC_glColor3i			1123
#define OPENGL_FUNC_glColor3s			1124
#define OPENGL_FUNC_glColor3ub			1125
#define OPENGL_FUNC_glColor3ui			1126
#define OPENGL_FUNC_glColor3us			1127
#define OPENGL_FUNC_glColor4b			1128
#define OPENGL_FUNC_glColor4d			1129
#define OPENGL_FUNC_glColor4f			1130
#define OPENGL_FUNC_glColor4i			1131
#define OPENGL_FUNC_glColor4s			1132
#define OPENGL_FUNC_glColor4ub			1133
#define OPENGL_FUNC_glColor4ui			1134
#define OPENGL_FUNC_glColor4us			1135
#define OPENGL_FUNC_glColor3bv			1136
#define OPENGL_FUNC_glColor3dv			1137
#define OPENGL_FUNC_glColor3fv			1138
#define OPENGL_FUNC_glColor3iv			1139
#define OPENGL_FUNC_glColor3sv			1140
#define OPENGL_FUNC_glColor3ubv			1141
#define OPENGL_FUNC_glColor3uiv			1142
#define OPENGL_FUNC_glColor3usv			1143
#define OPENGL_FUNC_glColor4bv			1144
#define OPENGL_FUNC_glColor4dv			1145
#define OPENGL_FUNC_glColor4fv			1146
#define OPENGL_FUNC_glColor4iv			1147
#define OPENGL_FUNC_glColor4sv			1148
#define OPENGL_FUNC_glColor4ubv			1149
#define OPENGL_FUNC_glColor4uiv			1150
#define OPENGL_FUNC_glColor4usv			1151
#define OPENGL_FUNC_glTexCoord1d		1152
#define OPENGL_FUNC_glTexCoord1f		1153
#define OPENGL_FUNC_glTexCoord1i		1154
#define OPENGL_FUNC_glTexCoord1s		1155
#define OPENGL_FUNC_glTexCoord2d		1156
#define OPENGL_FUNC_glTexCoord2f		1157
#define OPENGL_FUNC_glTexCoord2i		1158
#define OPENGL_FUNC_glTexCoord2s		1159
#define OPENGL_FUNC_glTexCoord3d		1160
#define OPENGL_FUNC_glTexCoord3f		1161
#define OPENGL_FUNC_glTexCoord3i		1162
#define OPENGL_FUNC_glTexCoord3s		1163
#define OPENGL_FUNC_glTexCoord4d		1164
#define OPENGL_FUNC_glTexCoord4f		1165
#define OPENGL_FUNC_glTexCoord4i		1166
#define OPENGL_FUNC_glTexCoord4s		1167
#define OPENGL_FUNC_glTexCoord1dv		1168
#define OPENGL_FUNC_glTexCoord1fv		1169
#define OPENGL_FUNC_glTexCoord1iv		1170
#define OPENGL_FUNC_glTexCoord1sv		1171
#define OPENGL_FUNC_glTexCoord2dv		1172
#define OPENGL_FUNC_glTexCoord2fv		1173
#define OPENGL_FUNC_glTexCoord2iv		1174
#define OPENGL_FUNC_glTexCoord2sv		1175
#define OPENGL_FUNC_glTexCoord3dv		1176
#define OPENGL_FUNC_glTexCoord3fv		1177
#define OPENGL_FUNC_glTexCoord3iv		1178
#define OPENGL_FUNC_glTexCoord3sv		1179
#define OPENGL_FUNC_glTexCoord4dv		1180
#define OPENGL_FUNC_glTexCoord4fv		1181
#define OPENGL_FUNC_glTexCoord4iv		1182
#define OPENGL_FUNC_glTexCoord4sv		1183
#define OPENGL_FUNC_glRasterPos2d		1184
#define OPENGL_FUNC_glRasterPos2f		1185
#define OPENGL_FUNC_glRasterPos2i		1186
#define OPENGL_FUNC_glRasterPos2s		1187
#define OPENGL_FUNC_glRasterPos3d		1188
#define OPENGL_FUNC_glRasterPos3f		1189
#define OPENGL_FUNC_glRasterPos3i		1190
#define OPENGL_FUNC_glRasterPos3s		1191
#define OPENGL_FUNC_glRasterPos4d		1192
#define OPENGL_FUNC_glRasterPos4f		1193
#define OPENGL_FUNC_glRasterPos4i		1194
#define OPENGL_FUNC_glRasterPos4s		1195
#define OPENGL_FUNC_glRasterPos2dv		1196
#define OPENGL_FUNC_glRasterPos2fv		1197
#define OPENGL_FUNC_glRasterPos2iv		1198
#define OPENGL_FUNC_glRasterPos2sv		1199
#define OPENGL_FUNC_glRasterPos3dv		1200
#define OPENGL_FUNC_glRasterPos3fv		1201
#define OPENGL_FUNC_glRasterPos3iv		1202
#define OPENGL_FUNC_glRasterPos3sv		1203
#define OPENGL_FUNC_glRasterPos4dv		1204
#define OPENGL_FUNC_glRasterPos4fv		1205
#define OPENGL_FUNC_glRasterPos4iv		1206
#define OPENGL_FUNC_glRasterPos4sv		1207
#define OPENGL_FUNC_glRectd			1208
#define OPENGL_FUNC_glRectf			1209
#define OPENGL_FUNC_glRecti			1210
#define OPENGL_FUNC_glRects			1211
#define OPENGL_FUNC_glRectdv			1212
#define OPENGL_FUNC_glRectfv			1213
#define OPENGL_FUNC_glRectiv			1214
#define OPENGL_FUNC_glRectsv			1215


/*
 * Vertex Arrays  (1.1)
 */

#define OPENGL_FUNC_glVertexPointer		1216
#define OPENGL_FUNC_glNormalPointer		1217
#define OPENGL_FUNC_glColorPointer		1218
#define OPENGL_FUNC_glIndexPointer		1219
#define OPENGL_FUNC_glTexCoordPointer		1220
#define OPENGL_FUNC_glEdgeFlagPointer		1221
#define OPENGL_FUNC_glGetPointerv		1222
#define OPENGL_FUNC_glArrayElement		1223
#define OPENGL_FUNC_glDrawArrays		1224
#define OPENGL_FUNC_glDrawElements		1225
#define OPENGL_FUNC_glInterleavedArrays		1226


 /*
 * Lighting
 */

#define OPENGL_FUNC_glShadeModel		1227
#define OPENGL_FUNC_glLightf			1228
#define OPENGL_FUNC_glLighti			1229
#define OPENGL_FUNC_glLightfv			1230
#define OPENGL_FUNC_glLightiv			1231
#define OPENGL_FUNC_glGetLightfv			1232
#define OPENGL_FUNC_glGetLightiv			1233
#define OPENGL_FUNC_glLightModelf		1234
#define OPENGL_FUNC_glLightModeli		1235
#define OPENGL_FUNC_glLightModelfv		1236
#define OPENGL_FUNC_glLightModeliv		1237
#define OPENGL_FUNC_glMaterialf			1238
#define OPENGL_FUNC_glMateriali			1239
#define OPENGL_FUNC_glMaterialfv			1240
#define OPENGL_FUNC_glMaterialiv			1241
#define OPENGL_FUNC_glGetMaterialfv		1242
#define OPENGL_FUNC_glGetMaterialiv		1243
#define OPENGL_FUNC_glColorMaterial		1244


/*
 * Raster functions
 */

#define OPENGL_FUNC_glPixelZoom			1245
#define OPENGL_FUNC_glPixelStoref		1246
#define OPENGL_FUNC_glPixelStorei		1247
#define OPENGL_FUNC_glPixelTransferf		1248
#define OPENGL_FUNC_glPixelTransferi		1249
#define OPENGL_FUNC_glPixelMapfv		1250
#define OPENGL_FUNC_glPixelMapuiv		1251
#define OPENGL_FUNC_glPixelMapusv		1252
#define OPENGL_FUNC_glGetPixelMapfv		1253
#define OPENGL_FUNC_glGetPixelMapuiv		1254
#define OPENGL_FUNC_glGetPixelMapusv		1255
#define OPENGL_FUNC_glBitmap			1256
#define OPENGL_FUNC_glReadPixels		1257
#define OPENGL_FUNC_glDrawPixels		1258
#define OPENGL_FUNC_glCopyPixels		1259


 /*
 * Stenciling
 */

#define OPENGL_FUNC_glStencilFunc		1260
#define OPENGL_FUNC_glStencilMask		1261
#define OPENGL_FUNC_glStencilOp			1262
#define OPENGL_FUNC_glClearStencil		1263


/*
 * Texture mapping
 */

#define OPENGL_FUNC_glTexGend			1264
#define OPENGL_FUNC_glTexGenf			1265
#define OPENGL_FUNC_glTexGeni			1266
#define OPENGL_FUNC_glTexGendv			1267
#define OPENGL_FUNC_glTexGenfv			1268
#define OPENGL_FUNC_glTexGeniv			1269
#define OPENGL_FUNC_glGetTexGendv		1270
#define OPENGL_FUNC_glGetTexGenfv		1271
#define OPENGL_FUNC_glGetTexGeniv		1272
#define OPENGL_FUNC_glTexEnvf			1273
#define OPENGL_FUNC_glTexEnvi			1274
#define OPENGL_FUNC_glTexEnvfv			1275
#define OPENGL_FUNC_glTexEnviv			1276
#define OPENGL_FUNC_glGetTexEnvfv		1277
#define OPENGL_FUNC_glGetTexEnviv		1278
#define OPENGL_FUNC_glTexParameterf		1279
#define OPENGL_FUNC_glTexParameteri		1280
#define OPENGL_FUNC_glTexParameterfv		1281
#define OPENGL_FUNC_glTexParameteriv		1282
#define OPENGL_FUNC_glGetTexParameterfv	1283
#define OPENGL_FUNC_glGetTexParameteriv	1284
#define OPENGL_FUNC_glGetTexLevelParameterfv	1285
#define OPENGL_FUNC_glGetTexLevelParameteriv	1286
#define OPENGL_FUNC_glTexImage1D		1287
#define OPENGL_FUNC_glTexImage2D		1288
#define OPENGL_FUNC_glGetTexImage		1289
/* 1.1 functions */
#define OPENGL_FUNC_glGenTextures		1290
#define OPENGL_FUNC_glDeleteTextures		1291
#define OPENGL_FUNC_glBindTexture		1292
#define OPENGL_FUNC_glPrioritizeTextures		1293
#define OPENGL_FUNC_glAreTexturesResident	1294
#define OPENGL_FUNC_glIsTexture			1295
#define OPENGL_FUNC_glTexSubImage1D		1296
#define OPENGL_FUNC_glTexSubImage2D		1297


/*
 * Evaluators
 */

#define OPENGL_FUNC_glCopyTexImage1D		1298
#define OPENGL_FUNC_glCopyTexImage2D		1299
#define OPENGL_FUNC_glCopyTexSubImage1D	1300
#define OPENGL_FUNC_glCopyTexSubImage2D	1301
#define OPENGL_FUNC_glMap1d			1302
#define OPENGL_FUNC_glMap1f			1303
#define OPENGL_FUNC_glMap2d			1304
#define OPENGL_FUNC_glMap2f			1305
#define OPENGL_FUNC_glGetMapdv			1306
#define OPENGL_FUNC_glGetMapfv			1307
#define OPENGL_FUNC_glGetMapiv			1308
#define OPENGL_FUNC_glEvalCoord1d		1309
#define OPENGL_FUNC_glEvalCoord1f		1310
#define OPENGL_FUNC_glEvalCoord1dv		1311
#define OPENGL_FUNC_glEvalCoord1fv		1312
#define OPENGL_FUNC_glEvalCoord2d		1313
#define OPENGL_FUNC_glEvalCoord2f		1314
#define OPENGL_FUNC_glEvalCoord2dv		1315
#define OPENGL_FUNC_glEvalCoord2fv		1316
#define OPENGL_FUNC_glMapGrid1d		1317
#define OPENGL_FUNC_glMapGrid1f		1318
#define OPENGL_FUNC_glMapGrid2d		1319
#define OPENGL_FUNC_glMapGrid2f		1320
#define OPENGL_FUNC_glEvalPoint1			1321
#define OPENGL_FUNC_glEvalPoint2			1322
#define OPENGL_FUNC_glEvalMesh1			1323
#define OPENGL_FUNC_glEvalMesh2			1324


 /*
 * Fog
 */

 #define OPENGL_FUNC_glFogf			1325
#define OPENGL_FUNC_glFogi			1326
#define OPENGL_FUNC_glFogfv			1327
#define OPENGL_FUNC_glFogiv			1328


 /*
 * Selection and Feedback
 */

#define OPENGL_FUNC_glFeedbackBuffer		1329
#define OPENGL_FUNC_glPassThrough		1330
#define OPENGL_FUNC_glSelectBuffer		1331
#define OPENGL_FUNC_glInitNames			1332
#define OPENGL_FUNC_glLoadName		1333
#define OPENGL_FUNC_glPushName			1334
#define OPENGL_FUNC_glPopName			1335


 /*
 * OpenGL 1.2
 */


 #define OPENGL_FUNC_glDrawRangeElements	1336
#define OPENGL_FUNC_glTexImage3D		1337
#define OPENGL_FUNC_glTexSubImage3D		1338
#define OPENGL_FUNC_glCopyTexSubImage3D	1339


 /*
 * GL_ARB_imaging
 */

#define OPENGL_FUNC_glColorTable			1340
#define OPENGL_FUNC_glColorSubTable			1341
#define OPENGL_FUNC_glColorTableParameteriv		1342
#define OPENGL_FUNC_glColorTableParameterfv		1343
#define OPENGL_FUNC_glCopyColorSubTable		1344
#define OPENGL_FUNC_glCopyColorTable			1345
#define OPENGL_FUNC_glGetColorTable			1346
#define OPENGL_FUNC_glGetColorTableParameterfv	1347
#define OPENGL_FUNC_glGetColorTableParameteriv	1348
#define OPENGL_FUNC_glBlendEquation			1349
#define OPENGL_FUNC_glBlendColor			1350
#define OPENGL_FUNC_glHistogram				1351
#define OPENGL_FUNC_glResetHistogram			1352
#define OPENGL_FUNC_glGetHistogram			1353
#define OPENGL_FUNC_glGetHistogramParameterfv	1354
#define OPENGL_FUNC_glGetHistogramParameteriv	1355
#define OPENGL_FUNC_glMinmax				1356
#define OPENGL_FUNC_glResetMinmax			1357
#define OPENGL_FUNC_glGetMinmax			1358
#define OPENGL_FUNC_glGetMinmaxParameterfv		1359
#define OPENGL_FUNC_glGetMinmaxParameteriv		1360
#define OPENGL_FUNC_glConvolutionFilter1D		1361
#define OPENGL_FUNC_glConvolutionFilter2D		1362
#define OPENGL_FUNC_glConvolutionParameterf		1363
#define OPENGL_FUNC_glConvolutionParameterfv		1364
#define OPENGL_FUNC_glConvolutionParameteri		1365
#define OPENGL_FUNC_glConvolutionParameteriv		1366
#define OPENGL_FUNC_glCopyConvolutionFilter1D		1367
#define OPENGL_FUNC_glCopyConvolutionFilter2D		1368
#define OPENGL_FUNC_glGetConvolutionFilter		1369
#define OPENGL_FUNC_glGetConvolutionParameterfv	1370
#define OPENGL_FUNC_glGetConvolutionParameteriv	1371
#define OPENGL_FUNC_glSeparableFilter2D			1372
#define OPENGL_FUNC_glGetSeparableFilter		1373


 /*
 * OpenGL 1.3
 */


#define OPENGL_FUNC_glActiveTexture			1374
#define OPENGL_FUNC_glClientActiveTexture		1375
#define OPENGL_FUNC_glCompressedTexImage1D		1376
#define OPENGL_FUNC_glCompressedTexImage2D		1377
#define OPENGL_FUNC_glCompressedTexImage3D		1378
#define OPENGL_FUNC_glCompressedTexSubImage1D	1379
#define OPENGL_FUNC_glCompressedTexSubImage2D	1380
#define OPENGL_FUNC_glCompressedTexSubImage3D	1381
#define OPENGL_FUNC_glGetCompressedTexImage	1382
#define OPENGL_FUNC_glMultiTexCoord1d			1383
#define OPENGL_FUNC_glMultiTexCoord1dv		1384
#define OPENGL_FUNC_glMultiTexCoord1f			1385
#define OPENGL_FUNC_glMultiTexCoord1fv			1386
#define OPENGL_FUNC_glMultiTexCoord1i			1387
#define OPENGL_FUNC_glMultiTexCoord1iv			1388
#define OPENGL_FUNC_glMultiTexCoord1s			1389
#define OPENGL_FUNC_glMultiTexCoord1sv		1390
#define OPENGL_FUNC_glMultiTexCoord2d			1391
#define OPENGL_FUNC_glMultiTexCoord2dv		1392
#define OPENGL_FUNC_glMultiTexCoord2f			1393
#define OPENGL_FUNC_glMultiTexCoord2fv			1394
#define OPENGL_FUNC_glMultiTexCoord2i			1395
#define OPENGL_FUNC_glMultiTexCoord2iv			1396
#define OPENGL_FUNC_glMultiTexCoord2s			1397
#define OPENGL_FUNC_glMultiTexCoord2sv		1398
#define OPENGL_FUNC_glMultiTexCoord3d			1399
#define OPENGL_FUNC_glMultiTexCoord3dv		1400
#define OPENGL_FUNC_glMultiTexCoord3f			1401
#define OPENGL_FUNC_glMultiTexCoord3fv			1402
#define OPENGL_FUNC_glMultiTexCoord3i			1403
#define OPENGL_FUNC_glMultiTexCoord3iv			1404
#define OPENGL_FUNC_glMultiTexCoord3s			1405
#define OPENGL_FUNC_glMultiTexCoord3sv		1406
#define OPENGL_FUNC_glMultiTexCoord4d			1407
#define OPENGL_FUNC_glMultiTexCoord4dv		1408
#define OPENGL_FUNC_glMultiTexCoord4f			1409
#define OPENGL_FUNC_glMultiTexCoord4fv			1410
#define OPENGL_FUNC_glMultiTexCoord4i			1411
#define OPENGL_FUNC_glMultiTexCoord4iv			1412
#define OPENGL_FUNC_glMultiTexCoord4s			1413
#define OPENGL_FUNC_glMultiTexCoord4sv		1414
#define OPENGL_FUNC_glLoadTransposeMatrixd		1415
#define OPENGL_FUNC_glLoadTransposeMatrixf		1416
#define OPENGL_FUNC_glMultTransposeMatrixd		1417
#define OPENGL_FUNC_glMultTransposeMatrixf		1418
#define OPENGL_FUNC_glSampleCoverage			1419


 /*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */

#define OPENGL_FUNC_glActiveTextureARB				1420
#define OPENGL_FUNC_glClientActiveTextureARB			1421
#define OPENGL_FUNC_glMultiTexCoord1dARB			1422
#define OPENGL_FUNC_glMultiTexCoord1dvARB			1423
#define OPENGL_FUNC_glMultiTexCoord1fARB			1424
#define OPENGL_FUNC_glMultiTexCoord1fvARB			1425
#define OPENGL_FUNC_glMultiTexCoord1iARB			1426
#define OPENGL_FUNC_glMultiTexCoord1ivARB			1427
#define OPENGL_FUNC_glMultiTexCoord1sARB			1428
#define OPENGL_FUNC_glMultiTexCoord1svARB			1429
#define OPENGL_FUNC_glMultiTexCoord2dARB			1430
#define OPENGL_FUNC_glMultiTexCoord2dvARB			1431
#define OPENGL_FUNC_glMultiTexCoord2fARB			1432
#define OPENGL_FUNC_glMultiTexCoord2fvARB			1433
#define OPENGL_FUNC_glMultiTexCoord2iARB			1434
#define OPENGL_FUNC_glMultiTexCoord2ivARB			1435
#define OPENGL_FUNC_glMultiTexCoord2sARB			1436
#define OPENGL_FUNC_glMultiTexCoord2svARB			1437
#define OPENGL_FUNC_glMultiTexCoord3dARB			1438
#define OPENGL_FUNC_glMultiTexCoord3dvARB			1439
#define OPENGL_FUNC_glMultiTexCoord3fARB			1440
#define OPENGL_FUNC_glMultiTexCoord3fvARB			1441
#define OPENGL_FUNC_glMultiTexCoord3iARB			1442
#define OPENGL_FUNC_glMultiTexCoord3ivARB			1443
#define OPENGL_FUNC_glMultiTexCoord3sARB			1444
#define OPENGL_FUNC_glMultiTexCoord3svARB			1445
#define OPENGL_FUNC_glMultiTexCoord4dARB			1446
#define OPENGL_FUNC_glMultiTexCoord4dvARB			1447
#define OPENGL_FUNC_glMultiTexCoord4fARB			1448
#define OPENGL_FUNC_glMultiTexCoord4fvARB			1449
#define OPENGL_FUNC_glMultiTexCoord4iARB			1450
#define OPENGL_FUNC_glMultiTexCoord4ivARB			1451
#define OPENGL_FUNC_glMultiTexCoord4sARB			1452
#define OPENGL_FUNC_glMultiTexCoord4svARB			1453

#define OPENGL_FUNC_glCreateDebugObjectMESA 	1454
#define OPENGL_FUNC_glClearDebugLogMESA 	1455
#define OPENGL_FUNC_glGetDebugLogMESA 	1456
#define OPENGL_FUNC_glGetDebugLogLengthMESA 	1457
#define OPENGL_FUNC_glProgramCallbackMESA	1458
#define OPENGL_FUNC_glGetProgramRegisterfvMESA	1459
#define OPENGL_FUNC_glFramebufferTextureLayerEXT	1460
#define OPENGL_FUNC_glBlendEquationSeparateATI	1461
#define OPENGL_FUNC_glEGLImageTargetTexture2DOES 	1462
#define OPENGL_FUNC_glEGLImageTargetRenderbufferStorageOES 	1463

#endif