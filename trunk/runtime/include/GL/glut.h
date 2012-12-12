
#include "gl.h"
#include "glu.h"

extern void *glutStrokeRoman;
extern void *glutStrokeMonoRoman;
extern void *glutBitmap9By15;
extern void *glutBitmap8By13;
extern void *glutBitmapTimesRoman10;
extern void *glutBitmapTimesRoman24;
extern void *glutBitmapHelvetica10;
extern void *glutBitmapHelvetica12;
extern void *glutBitmapHelvetica18;

void glutInit(int *pargc, char **argv);
void glutInitWindowPosition(int x, int y);
void glutInitWindowSize(int width, int height);
void glutInitDisplayMode(unsigned int displayMode);
void glutInitDisplayString(const char *displayMode);

void glutMainLoop(void);

int glutCreateWindow(const char *title);
int glutCreateSubWindow(int window, int x, int y, int width, int height);
void glutDestroyWindow(int window);
void glutSetWindow(int window);
int glutGetWindow(void);
void glutSetWindowTitle(const char *title);
void glutSetIconTitle(const char *title);
void glutReshapeWindow(int width, int height);
void glutPositionWindow(int x, int y);
void glutShowWindow(void);
void glutHideWindow(void);
void glutIconifyWindow(void);
void glutPushWindow(void);
void glutPopWindow(void);
void glutFullScreen(void);

void glutPostWindowRedisplay(int window);
void glutPostRedisplay(void);
void glutSwapBuffers(void);

void glutWarpPointer(int x, int y);
void glutSetCursor(int cursor);

void glutEstablishOverlay(void);
void glutRemoveOverlay(void);
void glutUseLayer(GLenum layer);
void glutPostOverlayRedisplay(void);
void glutPostWindowOverlayRedisplay(int window);
void glutShowOverlay(void);
void glutHideOverlay(void);

int glutCreateMenu(void (*callback) (int menu));
void glutDestroyMenu(int menu);
int glutGetMenu(void);
void glutSetMenu(int menu);
void glutAddMenuEntry(const char *label, int value);
void glutAddSubMenu(const char *label, int subMenu);
void glutChangeToMenuEntry(int item, const char *label, int value);
void glutChangeToSubMenu(int item, const char *label, int value);
void glutRemoveMenuItem(int item);
void glutAttachMenu(int button);
void glutDetachMenu(int button);

void glutTimerFunc(unsigned int time, void (*callback) (int), int value);
void glutIdleFunc(void (*callback) (void));

void glutKeyboardFunc(void (*callback) (unsigned char, int, int));
void glutSpecialFunc(void (*callback) (int, int, int));
void glutReshapeFunc(void (*callback) (int, int));
void glutVisibilityFunc(void (*callback) (int));
void glutDisplayFunc(void (*callback) (void));
void glutMouseFunc(void (*callback) (int, int, int, int));
void glutMotionFunc(void (*callback) (int, int));
void glutPassiveMotionFunc(void (*callback) (int, int));
void glutEntryFunc(void (*callback) (int));

void glutKeyboardUpFunc(void (*callback) (unsigned char, int, int));
void glutSpecialUpFunc(void (*callback) (int, int, int));
void glutJoystickFunc(void (*callback) (unsigned int, int, int, int),
	int pollInterval);
void glutMenuStateFunc(void (*callback) (int));
void glutMenuStatusFunc(void (*callback) (int, int, int));
void glutOverlayDisplayFunc(void (*callback) (void));
void glutWindowStatusFunc(void (*callback) (int));

void glutSpaceballMotionFunc(void (*callback) (int, int, int));
void glutSpaceballRotateFunc(void (*callback) (int, int, int));
void glutSpaceballButtonFunc(void (*callback) (int, int));
void glutButtonBoxFunc(void (*callback) (int, int));
void glutDialsFunc(void (*callback) (int, int));
void glutTabletMotionFunc(void (*callback) (int, int));
void glutTabletButtonFunc(void (*callback) (int, int, int, int));

int glutGet(GLenum query);
int glutDeviceGet(GLenum query);
int glutGetModifiers(void);
int glutLayerGet(GLenum query);

void glutBitmapCharacter(void *font, int character);
int glutBitmapWidth(void *font, int character);
void glutStrokeCharacter(void *font, int character);
int glutStrokeWidth(void *font, int character);
int glutBitmapLength(void *font, const unsigned char *string);
int glutStrokeLength(void *font, const unsigned char *string);

void glutWireCube(GLdouble size);
void glutSolidCube(GLdouble size);
void glutWireSphere(GLdouble radius, GLint slices, GLint stacks);
void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks);
void glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
void glutSolidCone(GLdouble base, GLdouble height, GLint slices,
	GLint stacks);

void glutWireTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides,
	GLint rings);
void glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides,
	GLint rings);
void glutWireDodecahedron(void);
void glutSolidDodecahedron(void);
void glutWireOctahedron(void);
void glutSolidOctahedron(void);
void glutWireTetrahedron(void);
void glutSolidTetrahedron(void);
void glutWireIcosahedron(void);
void glutSolidIcosahedron(void);

void glutWireTeapot(GLdouble size);
void glutSolidTeapot(GLdouble size);

void glutGameModeString(const char *string);
int glutEnterGameMode(void);
void glutLeaveGameMode(void);
int glutGameModeGet(GLenum query);

int glutVideoResizeGet(GLenum query);
void glutSetupVideoResizing(void);
void glutStopVideoResizing(void);
void glutVideoResize(int x, int y, int width, int height);
void glutVideoPan(int x, int y, int width, int height);

void glutSetColor(int color, GLfloat red, GLfloat green, GLfloat blue);
GLfloat glutGetColor(int color, int component);
void glutCopyColormap(int window);

void glutIgnoreKeyRepeat(int ignore);
void glutSetKeyRepeat(int repeatMode);
void glutForceJoystickFunc(void);

int glutExtensionSupported(const char *extension);
void glutReportErrors(void);
