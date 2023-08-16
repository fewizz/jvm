package jvm;

public class AppClassLoader extends ClassLoader {

	private AppClassLoader() {}

	public static final ClassLoader INSTANCE = new AppClassLoader();

	@Override
	native protected Class<?> loadClass(
		String name,
		boolean resolve
	);

	@Override
	native protected Class<?> findClass(String name)
		throws ClassNotFoundException;

}
