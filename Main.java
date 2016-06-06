/* Author: Farrell Farahbod <farrellfarahbod@gmail.com>
 * License: public domain
 * 
 * Some notes:
 * 
 * This is my first Java3D program. It's probably not very good, but it's just a quick test for a 9DOF sensor module.
 * 
 * 1. Install Java3D from http://www.oracle.com/technetwork/articles/javase/index-jsp-138252.html
 * 2. Make a Java Project in Eclipse.
 * 3. Follow these directions to tell Eclipse about Java3D: https://www.cs.utexas.edu/~scottm/cs324e/handouts/setUpJava3dEclipse.htm
 * 4. Copy the code below into a Main class in your Project.
 * 5. Add jSerialComm to the Eclipse Project as well: http://fazecast.github.io/jSerialComm/
 * 
 */

import java.util.Scanner;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Canvas3D;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.swing.JFrame;
import javax.vecmath.Quat4d;
import javax.vecmath.Vector3d;
import com.fazecast.jSerialComm.SerialPort;
import com.sun.j3d.utils.geometry.ColorCube;
import com.sun.j3d.utils.universe.SimpleUniverse;

public class Main {

	public static void main(String[] args) {
		
		JFrame frame = new JFrame("Sensor Fusion Visual Test Program");
		Canvas3D canvas = new Canvas3D(SimpleUniverse.getPreferredConfiguration());
		SimpleUniverse universe = new SimpleUniverse(canvas);
		BranchGroup group = new BranchGroup();
		ColorCube cube = new ColorCube(0.3);
		
		TransformGroup transformGroup = new TransformGroup();
		transformGroup.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
		transformGroup.addChild(cube);
		
		universe.getViewingPlatform().setNominalViewingTransform();
		group.addChild(transformGroup);
		universe.addBranchGraph(group);
		
		frame.add(canvas);
		frame.setSize(800, 600);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setVisible(true);
		
		SerialPort port = SerialPort.getCommPort("COM10");
		port.setBaudRate(1000000);
		port.setComPortTimeouts(SerialPort.TIMEOUT_SCANNER, 1, 1);
		if(port.openPort() == false) {
			System.err.println("Unable to open the serial port. Exiting.");
			System.exit(1);
		}
		Scanner s = new Scanner(port.getInputStream());
		while(s.hasNextLine()) {
			try {
				String line = s.nextLine();
				String[] token = line.split(" ");
				
				// multiply x/y/z by -1 to swap frames of reference
				double w = Double.parseDouble(token[0]);
				double x = -Double.parseDouble(token[1]);
				double y = -Double.parseDouble(token[2]);
				double z = -Double.parseDouble(token[3]);
				
				Quat4d quaternion = new Quat4d(w, x, y, z);
				Vector3d vector = new Vector3d(0.0, 0.0, 0.0);
				transformGroup.setTransform(new Transform3D(quaternion, vector, 1.0));
				
				// the inverse cosine of w gives you the pitch *if* you normalize the quaternion with x and z being zero
				double pitch = Math.acos(w / Math.sqrt(w*w + y*y)) * 2.0 - (Math.PI / 2.0);
				
				System.out.println(String.format("w = %+2.3f     x = %+2.3f     y = %+2.3f     z = %+2.3f     pitch = %+1.3f", w, x, y, z, pitch));
			} catch(Exception e) {}
		}
		s.close();
		System.err.println("Lost communication with the serial port. Exiting.");
		System.exit(1);
		
	}

}
