package de.ferroequinologist.robosimandroid;

public class Controller
{
	private RoboSim activity;
	
	// Native interface
	private int nativeIndex;

	private static native int load(Controller javaController, int mode, int flags, String path, String address, String port, String assetArchivePath);
	private static native void destroy(int index);
	private static native void shutDown(int index);
	private static native void keyUp(int index, int character, boolean isPrintable);
	private static native void keyDown(int index, int character, boolean isPrintable);
	private static native void initVideo(int index);
	private static native void initAudio(int index);
	private static native void draw(int index);
	private static native void resize(int index, int width, int height);
	private static native void update(int index, float timeDelta);
	private static native void setScreenOrientation(int index, int orientation);
	private static native void scroll(int index, int amount);
	private static native void mouseDown(int index, float x, float y, int button);
	private static native void mouseUp(int index, float x, float y, int button);
	private static native void rotateCamera(int index, float radians);
	private static native void moveCamera(int index, float x, float y);
	private static native void touchDown(int index, int touchIndex, float positionX, float positionY);
	private static native void touchMoved(int index, int touchIndex, float positionX, float positionY);
	private static native void touchUp(int index, int touchIndex);
	private static native void touchCancelled(int index, int touchIndex);
	private static native void fileChosen(String filename);
	
	static
	{		
		System.loadLibrary("GLESv1_CM");
		System.loadLibrary("log");
		System.loadLibrary("z");
		System.loadLibrary("robosim");
	}

	// Enums
	// NetworkMode, for Controller(mode, flags, path, address, port)
	public static final int LetUserChoose = 0;
	public static final int SingleMode = 1;
	public static final int ClientMode = 2;
	public static final int ServerMode = 3;

	// Flags, for Controller(mode, flags, path, address, port)
	public static final int ClientFlagUIOnServer = 1 << 0;
	public static final int ServerFlagNoBroadcast = 1 << 0;
	
	// Unprintable keys, for keyDown and keyUp
	public static final int UpArrow = 0;
	public static final int DownArrow = 1;
	public static final int LeftArrow = 2;
	public static final int RightArrow = 3;
	
	// Screen orientation, for setScreenOrientation
	public static final int Normal = 0;
	public static final int RotatedLeft = 1;
	public static final int RotatedRight = 2;
	public static final int UpsideDown = 3;
	
	// Public access
	public Controller(RoboSim anActivity, int mode, int flags, String path, String address, String port, String assetArchivePath)
	{
		activity = anActivity;
		nativeIndex = load(this, mode, flags, path, address, port, assetArchivePath);
	}
	
	public void finalize()
	{
		destroy(nativeIndex);
	}
	
	public void shutDown()
	{
		shutDown(nativeIndex);
	}
	
	public void keyUp(int character, boolean isPrintable)
	{
		keyUp(nativeIndex, (char) character, isPrintable);
	}
	
	public void keyDown(int character, boolean isPrintable)
	{
		keyDown(nativeIndex, (char) character, isPrintable);
	}
	
	public void initVideo()
	{

		initVideo(nativeIndex);
	}
	
	public void initAudio()
	{
		initAudio(nativeIndex);
	}
	
	public void draw()
	{
		draw(nativeIndex);
	}
	
	public void resize(int width, int height)
	{
		resize(nativeIndex, width, height);
	}
	
	public void update(float timeDelta)
	{
		update(nativeIndex, timeDelta);
	}
	
	public void setScreenOrientation(int orientation)
	{
		setScreenOrientation(nativeIndex, orientation);
	}
	
	public void scroll(int amount)
	{
		scroll(nativeIndex, amount);
	}
	
	public void mouseDown(float x, float y, int button)
	{
		mouseDown(nativeIndex, x, y, button);
	}
	
	public void mouseUp(float x, float y, int button)
	{
		mouseUp(nativeIndex, x, y, button);
	}
	
	public void rotateCamera(float radians)
	{
		rotateCamera(nativeIndex, radians);
	}
	
	public void moveCamera(float x, float y)
	{
		moveCamera(nativeIndex, x, y);
	}
	
	public void touchDown(int touchIndex, float positionX, float positionY)
	{
		touchDown(nativeIndex, touchIndex, positionX, positionY);
	}
	
	public void touchMoved(int touchIndex, float positionX, float positionY)
	{
		touchMoved(nativeIndex, touchIndex, positionX, positionY);
	}
	
	public void touchUp(int touchIndex)
	{
		touchUp(nativeIndex, touchIndex);
		
	}
	
	public void touchCancelled(int touchIndex)
	{
		touchCancelled(nativeIndex, touchIndex);
	}

	// To be called from native code
	public void startFileChooser()
	{
		activity.startFileChooser();
	}
	
	public void fileFound(String filename)
	{
		fileChosen(filename);
	}
}
