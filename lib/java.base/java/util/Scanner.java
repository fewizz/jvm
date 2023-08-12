package java.util;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;

public class Scanner { // implements Iterator<String>, Closeable
	private static final int BUFFER_SIZE = 1024;
	private final StringBuilder sb_ = new StringBuilder();

	private final InputStream is_;
	private final CharsetDecoder decoder_;

	private final ByteBuffer buffer_;
	private final CharBuffer chars_buffer_;

	public Scanner(InputStream source) {
		this.is_ = source;
		this.decoder_ = Charset.defaultCharset().newDecoder();
		this.buffer_ = ByteBuffer.allocate(BUFFER_SIZE);
		this.chars_buffer_ = CharBuffer.allocate(
			(int)(decoder_.maxCharsPerByte() * BUFFER_SIZE + 1.0F)
		);
	}

	public String next() throws IOException {
		while(true) {
			{
				int index = sb_.indexOf("\n");
				if (index != -1) {
					String result = sb_.substring(0, index);
					sb_.delete(0, index + 1);
					return result;
				}
			}

			int read = is_.read(
				buffer_.array(),
				buffer_.arrayOffset(),
				buffer_.limit()
			);
			buffer_.limit(read);

			decoder_.decode(buffer_, chars_buffer_, false);
			buffer_.clear();

			sb_.append(
				chars_buffer_.array(),
				chars_buffer_.arrayOffset(),
				chars_buffer_.position()
			);
			chars_buffer_.position(0);
		}
	}

}