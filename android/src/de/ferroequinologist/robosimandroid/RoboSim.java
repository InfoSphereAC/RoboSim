package de.ferroequinologist.robosimandroid;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.Window;
import android.view.WindowManager;

public class RoboSim extends Activity
{
	private RoboSimGLSurfaceView glView;
	private Controller controller;
	private OrientationEventListener orientationListener;
    
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
    	super.onCreate(savedInstanceState);
    	requestWindowFeature(Window.FEATURE_NO_TITLE);
    	
    	String assetPath = null;
		try {
			assetPath = getPackageManager().getPackageInfo(getComponentName().getPackageName(), 0).applicationInfo.sourceDir;
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		controller = new Controller(this, Controller.LetUserChoose, 0, null, null, null, assetPath);
        glView = new RoboSimGLSurfaceView(this);
        setContentView(glView);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        		WindowManager.LayoutParams.FLAG_FULLSCREEN);
        orientationListener = new OrientationEventListener(this, SensorManager.SENSOR_DELAY_NORMAL) {
            @Override
            public void onOrientationChanged(int orientation)
            {
            	if (orientation == ORIENTATION_UNKNOWN) return;
            	
            	int orientationConstant = 0;
            	if (orientation < 45 || orientation >= 315)
            		orientationConstant = Controller.Normal;
            	else if (orientation >= 45 && orientation < 135)
            		orientationConstant = Controller.RotatedRight;
            	else if (orientation >= 135 && orientation < 225)
            		orientationConstant = Controller.UpsideDown;
            	else if (orientation >= 225 && orientation < 315)
            		orientationConstant = Controller.RotatedRight;
            	
            	controller.setScreenOrientation(orientationConstant);
            }
        };
    }
    
    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
    	super.onConfigurationChanged(newConfig);
    }
    
    public Controller getController()
    {
    	return controller;
    }
    
    @Override
    protected void onPause()
    {
        super.onPause();
        glView.onPause();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        glView.onResume();
    }
    
    public void startFileChooser()
    {
    	Intent intent = new Intent(this, FileChooser.class);
    	startActivityForResult(intent, 210);
    }
    
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
    	if (resultCode == RESULT_CANCELED) return;
    	
    	controller.fileFound(data.toString());
    }
}

class RoboSimUpdateRunnable implements Runnable
{
	Controller controller;
	Handler handler;
	RoboSimGLSurfaceView glView;
	long lastTime;
	
	public RoboSimUpdateRunnable(Controller aController, RoboSimGLSurfaceView view)
	{
		controller = aController;
		glView = view;
		handler = new Handler();
	}
	
	public void start()
	{
		lastTime = System.nanoTime();
		controller.update(0);
		handler.postDelayed(this, 1000 / 60);
	}
	
	public void stop()
	{
		handler.removeCallbacks(this);
	}
	
	@Override
	public void run()
	{
		long now = System.nanoTime();
		final float diffInSeconds = (float) (now - lastTime) * 1e-9f;
		lastTime = now;
		
		controller.update(diffInSeconds);
		handler.postDelayed(this, 1000 / 60);
	}
}

class RoboSimGLSurfaceView extends GLSurfaceView
{
	private Controller controller;
    private RoboSimUpdateRunnable updater;
    private RoboSim activity;
    
	public RoboSimGLSurfaceView(RoboSim context)
	{
		super(context);
		activity = context;
		controller = context.getController();
        updater = new RoboSimUpdateRunnable(controller, this);
  
		setRenderer(new RoboSimRenderer(controller, updater));
		setFocusableInTouchMode(true);
	}
	
	@Override
	public boolean onTouchEvent(final MotionEvent event)
	{		
		for (int i = 0; i < event.getPointerCount(); i++)
		{
			float x = event.getX(i);
			float y = event.getY(i);
			int id = event.getPointerId(i);
			int action = event.getActionMasked();
			
			// ACTION_{DOWN|UP} is for the first touch. Any others are ACTION_POINTER_{DOWN|UP} (no such thing for move or cancel, btw).
			if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN)
				controller.touchDown(id, x, y);
			else if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_POINTER_UP)
				controller.touchUp(id);
			else if (action == MotionEvent.ACTION_MOVE)
				controller.touchMoved(id, x, y);
			else if (action == MotionEvent.ACTION_CANCEL)
				controller.touchCancelled(id);
		}
		
		return true;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{		
		switch(keyCode)
		{
		case KeyEvent.KEYCODE_DPAD_DOWN:
			controller.keyDown(Controller.DownArrow, false);
			break;
		case KeyEvent.KEYCODE_DPAD_LEFT:
			controller.keyDown(Controller.LeftArrow, false);
			break;
		case KeyEvent.KEYCODE_DPAD_RIGHT:
			controller.keyDown(Controller.RightArrow, false);
			break;
		case KeyEvent.KEYCODE_DPAD_UP:
			controller.keyDown(Controller.UpArrow, false);
			break;
		}
		
		controller.keyDown(event.getUnicodeChar(), true);
		
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		switch(keyCode)
		{
		case KeyEvent.KEYCODE_DPAD_DOWN:
			controller.keyUp(Controller.DownArrow, false);
			break;
		case KeyEvent.KEYCODE_DPAD_LEFT:
			controller.keyUp(Controller.LeftArrow, false);
			break;
		case KeyEvent.KEYCODE_DPAD_RIGHT:
			controller.keyUp(Controller.RightArrow, false);
			break;
		case KeyEvent.KEYCODE_DPAD_UP:
			controller.keyUp(Controller.UpArrow, false);
			break;
		}
		
		controller.keyUp(event.getUnicodeChar(), true);
		return true;
	}
	
	@Override
	public void onPause()
	{
		super.onPause();
		updater.stop();
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
		updater.start(); 
	}
}

class RoboSimRenderer implements GLSurfaceView.Renderer
{
	private Controller controller;
    private RoboSimUpdateRunnable updater;
    
	RoboSimRenderer(Controller aController, RoboSimUpdateRunnable anUpdater)
	{
		controller = aController;
		updater = anUpdater;
	}
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		controller.initVideo();
		updater.start();
	}
	public void onSurfaceChanged(GL10 gl, int width, int height)
	{
		controller.resize(width, height);
	}
	public void onDrawFrame(GL10 gl)
	{
		controller.draw();
	}
}