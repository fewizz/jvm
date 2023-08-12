public class T07_Array {

	public static void main(String... args) throws Exception {
		int arr[] = new int[128];
		if(arr.length != 128) {
			throw new Exception();
		}

		for(int x = 0; x < arr.length; ++x) {
			if(arr[x] != 0) {
				throw new Exception();
			}
		}

		for(int x = 0; x < arr.length; ++x) {
			arr[x] = x;
		}

		for(int x = 0; x < arr.length; ++x) {
			if(arr[x] != x) {
				throw new Exception();
			}
		}

		boolean outOfBounds = false;

		{
			try {
				arr[127] = 42;
			}
			catch(IndexOutOfBoundsException e) {
				outOfBounds = true;
			}
			if(outOfBounds) {
				throw new Exception();
			}
		}

		{
			outOfBounds = false;
			try {
				arr[128] = 42;
			}
			catch(IndexOutOfBoundsException e) {
				outOfBounds = true;
			}
			if(!outOfBounds) {
				throw new Exception();
			}
		}

		{
			outOfBounds = false;
			try {
				arr[-1] = 42;
			}
			catch(IndexOutOfBoundsException e) {
				outOfBounds = true;
			}
			if(!outOfBounds) {
				throw new Exception();
			}
		}

		{
			// i took it from Zombob
			if(map[0][0] != '-') throw new Exception();
			if(map[1][0] != 'Z') throw new Exception();
			if(map[0][1] != 'S') throw new Exception();
			if(map[1][1] != '-') throw new Exception();
		}
	}

	public static char[][] map = {
		{'-', 'S', 'E', 'Z', 'Z', '-'},
		{'Z', '-', 'Z', '-', 'S', '-'},
		{'S', 'S', 'E', 'S', '-', 'Z'},
		{'E', 'Z', 'S', 'Z', 'E', '-'},
		{'Z', '-', 'Z', '-', 'S', 'S'},
		{'S', 'S', '-', 'E', 'Z', '-'},
	};

}