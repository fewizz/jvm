package java.util;

public class Hashtable<K,V>
	extends Dictionary<K,V>
	implements Map<K,V>//, Cloneable, Serializable
{
	private int size_ = 0;
	private int capacity_;
	private float loadFactor_;
	private Entry<K, V> entries_[];

	static class Entry<K, V> implements Map.Entry<K, V> {
		final K key_;
		final V value_;
		final int keyHash_;
		final int valueHash_;
		Entry<K, V> next_ = null;

		Entry(K key, V value, int keyHash, int valueHash) {
			this.key_ = key;
			this.value_ = value;
			this.keyHash_ = keyHash;
			this.valueHash_ = valueHash;
		}

		@Override
		public K getKey() { return key_; }

		@Override
		public V getValue() { return value_; }

		@Override
		public V setValue(V value) {
			throw new UnsupportedOperationException();
		}

		@Override
		public boolean equals(Object o) {
			Entry e = (Entry) o;
			return (
				this.getKey() == null ?
				e.getKey() == null :
				this.getKey().equals(e.getKey())
			) && (
				this.getValue() == null ?
				e.getValue() == null :
				this.getValue().equals(e.getValue())
			);
		}

		@Override
		public int hashCode() {
			return keyHash_ ^ valueHash_;
		}
	} // class Entry<K, V>

	@SuppressWarnings("unchecked")
	public Hashtable(int initialCapacity, float loadFactor) {
		if(
			initialCapacity < 0 ||
			Float.isNaN(loadFactor) ||
			loadFactor <= 0.0F
		) {
			throw new IllegalArgumentException();
		}
		this.capacity_ = initialCapacity == 0 ? 1 : initialCapacity;
		this.loadFactor_ = loadFactor;

		this.entries_ = new Entry[initialCapacity];
	}

	public Hashtable(int initialCapacity) {
		this(initialCapacity, 0.75F);
	}

	public Hashtable() {
		this(11, 0.75F);
	}

	@Override
	public synchronized int size() {
		return size_;
	}

	@Override
	public synchronized boolean isEmpty() {
		return size_ == 0;
	}

	//@Override
	//public synchronized Enumeration<K> keys() {
	//	return null; //TODO
	//}

	//@Override
	//public synchronized Enumeration<V> elements() {
	//	return null; // TODO
	//}

	public synchronized boolean contains(Object value) {
		return containsValue(value);
	}

	@Override
	public boolean containsValue(Object value) {
		if(value == null) {
			throw new NullPointerException();
		}
		for(int i = 0; i < entries_.length; ++i) {
			Entry e = entries_[i];
			while(e != null) {
				if(e.value_.equals(value)) {
					return true;
				}
				e = e.next_;
			}
		}
		return false;
	}

	@Override
	public synchronized boolean containsKey(Object key) {
		int hash = key.hashCode();
		int index = (hash & 0x7FFFFFFF) % entries_.length;

		Entry<K, V> e = entries_[index];
		while(e != null) {
			if(e.keyHash_ == hash && e.key_.equals(key)) {
				return true;
			}
			e = e.next_;
		}
		return false;
	}

	@Override
	public synchronized V get(Object key) {
		int hash = key.hashCode();
		int index = (hash & 0x7FFFFFFF) % entries_.length;

		Entry<K, V> e = entries_[index];
		while(e != null) {
			if ((e.keyHash_ == hash) && e.key_.equals(key)) {
				return (V)e.value_;
			}
			e = e.next_;
		}
		return null;
	}

	protected void rehash() {
		@SuppressWarnings({"unchecked"})
		Entry<K, V> newEntries[] = new Entry[entries_.length * 2 + 1];

		for(int i = 0; i < entries_.length; ++i) {
			Entry<K, V> e = entries_[i];

			while(e != null) {
				Entry<K, V> next = e.next_;
				e.next_ = null;

				int index = (e.keyHash_ % 0x7FFFFFFF) % newEntries.length;
				Entry<K, V> new_e = newEntries[index];
				e.next_ = new_e;
				newEntries[index] = e;
				e = next;
			}
		}

		this.entries_ = newEntries;
	}

	@Override
	public synchronized V put(K key, V value) {
		if(value == null) {
			throw new NullPointerException();
		}
		if(size_ > entries_.length * loadFactor_) {
			rehash();
		}

		int keyHash = key.hashCode();
		int index = (keyHash & 0x7FFFFFFF) % entries_.length;

		Entry<K, V> e = entries_[index];
		var new_e = new Entry<K, V>(key, value, keyHash, value.hashCode());

		if(e == null) {
			entries_[index] = new_e;
			++size_;
			return null;
		}

		Entry<K, V> prev_e = null;

		do {
			if (e.keyHash_ == keyHash && e.key_.equals(key)) {
				V prev_value = e.value_;
				if(prev_e != null) {
					prev_e.next_ = new_e;
				}
				return prev_value;
			}

			prev_e = e;
			e = e.next_;
		}
		while(e != null);

		++size_;
		prev_e.next_ = new_e;

		return null;
	}

	@Override
	public synchronized V remove(Object key) {
		int keyHash = key.hashCode();
		int index = (keyHash & 0x7FFFFFFF) % entries_.length;

		Entry<K, V> e = entries_[index];
		Entry<K, V> prev_e = null;
		while(e != null) {
			if(e.keyHash_ == keyHash && e.key_.equals(key)) {
				if(prev_e == null) {
					entries_[index] = null;
				}
				else {
					prev_e.next_ = e.next_;
				}
				return e.value_;
			}
			prev_e = e;
			e = e.next_;
		}
		return null;
	}

	@Override
	public void clear() {
		for(int i = 0; i < entries_.length; ++i) {
			this.entries_[i] = null;
		}
	}

}