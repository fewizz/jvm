package java.lang;

public class Class<T> {
	
	public String getName() {
		return __getName();
	}

	private native String __getName();

}