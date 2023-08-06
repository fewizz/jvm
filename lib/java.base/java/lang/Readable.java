package java.lang;

import java.nio.CharBuffer;
import java.io.IOException;

public interface Readable {

	int read(CharBuffer cb) throws IOException;

}