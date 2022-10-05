import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;

public class T14_UTF8Charset {

	public static void main(String... args) {
		Charset utf8 = Charset.defaultCharset();
		CharsetDecoder d = utf8.newDecoder();

		{
			ByteBuffer bb = ByteBuffer.allocate(1);
			bb.put(0, (byte) 'a');
			CharBuffer out = CharBuffer.allocate(1);
			CoderResult res = d.decode(bb, out, true);
			if(res != CoderResult.UNDERFLOW) {
				System.exit(1);
			}
			if(out.position() != 1) {
				System.exit(2);
			}
			if(out.get(0) != 'a') {
				System.exit(3);
			}
		}
		{
			ByteBuffer bb = ByteBuffer.allocate(2);
			// ы
			bb.put(0, (byte) 0xD1);
			bb.put(1, (byte) 0x8B);
			CharBuffer out = CharBuffer.allocate(1);
			CoderResult res = d.decode(bb, out, true);
			if(res != CoderResult.UNDERFLOW) {
				System.exit(4);
			}
			if(out.position() != 1) {
				System.exit(5);
			}
			if(out.get(0) != 'ы') {
				System.exit(6);
			}
		}
		{
			ByteBuffer bb = ByteBuffer.allocate(4);
			// 🤔
			bb.put(0, (byte) 0xF0);
			bb.put(1, (byte) 0x9F);
			bb.put(2, (byte) 0xA4);
			bb.put(3, (byte) 0x94);
			CharBuffer out = CharBuffer.allocate(2);
			CoderResult res = d.decode(bb, out, true);
			if(res != CoderResult.UNDERFLOW) {
				System.exit(7);
			}
			if(out.position() != 2) {
				System.exit(8);
			}
			if(!((int)out.get(0) == 0xD83E && (int)out.get(1) == 0xDD14)) {
				System.exit(9);
			}
		}
	}

}