public class T11_ArrayCopy {

	public static void main(String... args) throws Exception {
		{
			int[] src = new int[16];
			for(int i = 0; i < 16; ++i) { src[i] = i + 10; }
			int[] dst = new int[16];
			System.arraycopy(src, 0, dst, 0, 0);
			System.arraycopy(src, 0, dst, 0, 8);
			System.arraycopy(src, 0, dst, 0, 16);

			for(int i = 0; i < 16; ++i) {
				if(dst[i] != i + 10) throw new Exception();
			}

			boolean caughtIOOB = false;
			try {
				System.arraycopy(src, 0, dst, 0, 17);
			} catch(IndexOutOfBoundsException e) {
				caughtIOOB = true;
			}
			if(!caughtIOOB) throw new Exception();

			boolean caughtAS = false;
			float[] dstF = new float[16];
			try {
				System.arraycopy(src, 0, dstF, 0, 16);
			} catch(ArrayStoreException e) {
				caughtAS = true;
			}
			if(!caughtAS) throw new Exception();
		}
		{ // copy to self
			int[] src = new int[16];
			for(int i = 0; i < 16; ++i) { src[i] = i; }

			System.arraycopy(src, 0, src, 8, 8);
			for(int i = 0; i < 16; ++i) {
				if(src[i] != i % 8) throw new Exception();
			}

			for(int i = 0; i < 16; ++i) { src[i] = i; }

			System.arraycopy(src, 8, src, 0, 8);
			for(int i = 0; i < 16; ++i) {
				if(src[i] != (i % 8) + 8) throw new Exception();
			}
		}
	}

}