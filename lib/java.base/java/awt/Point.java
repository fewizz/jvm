package java.awt;

public class Point {

	public int x = 0;
	public int y = 0;

	public Point() {}

	public Point(int x, int y) {
		this.x = x;
		this.y = y;
	}

	@Override
	public Object clone() {
		return new Point(this.x, this.y);
	}

}