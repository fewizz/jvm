import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileDescriptor;

public class HelloWorldFOS {

	public static void main(String... args) throws IOException {
		FileOutputStream out = new FileOutputStream(FileDescriptor.out);
		out.write(new byte[] {
			'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!'
		});
	}

}