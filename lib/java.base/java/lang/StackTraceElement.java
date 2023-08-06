package java.lang;

import java.io.Serializable;
import java.util.Objects;

public final class StackTraceElement implements Serializable {

	private final String
		classLoaderName_,
		moduleName_,
		moduleVersion_,
		declaringClass_,
		methodName_,
		fileName_;

	private final int lineNumber_;

	public StackTraceElement(
		String declaringClass, String methodName,
		String fileName, int lineNumber
	) {
		this.classLoaderName_ = null;
		this.moduleName_      = null;
		this.moduleVersion_   = null;
		this.declaringClass_  = declaringClass;
		this.methodName_      = methodName;
		this.fileName_        = fileName;
		this.lineNumber_      = lineNumber;
	}

	public StackTraceElement(
		String classLoaderName,
		String moduleName, String moduleVersion,
		String declaringClass, String methodName,
		String fileName, int lineNumber
	) {
		this.classLoaderName_ = classLoaderName;
		this.moduleName_      = moduleName;
		this.moduleVersion_   = moduleVersion;
		this.declaringClass_  = declaringClass;
		this.methodName_      = methodName;
		this.fileName_        = fileName;
		this.lineNumber_      = lineNumber;
	}

	public String getFileName() {
		return this.fileName_;
	}

	public int getLineNumber() {
		return this.lineNumber_;
	}

	public String getModuleName() {
		return this.moduleName_;
	}

	public String getModuleVersion() {
		return this.moduleVersion_;
	}

	public String getClassLoaderName() {
		return this.classLoaderName_;
	}

	public String getClassName() {
		return this.declaringClass_;
	}

	public String getMethodName() {
		return this.methodName_;
	}

	public boolean isNativeMethod() {
		return this.lineNumber_ == -2;
	}

	@Override
	public java.lang.String toString() {
		StringBuilder sb = new StringBuilder(64);

		if(classLoaderName_ != null) {
			sb.append(classLoaderName_);
			sb.append('/');
		}

		if(moduleName_ != null && moduleVersion_ != null) {
			sb.append(moduleName_);
			sb.append("@");
			sb.append(moduleVersion_);
		}

		sb.append(declaringClass_);
		sb.append('.');
		sb.append(methodName_);

		sb.append('(');
		if(fileName_ != null) {
			sb.append(fileName_);
			if(lineNumber_ >= 0) {
				sb.append(':');
				sb.append(lineNumber_);
			}
		}
		else {
			sb.append(isNativeMethod() ? "Native Method" : "Unknown Source");
		}
		sb.append(')');

		return sb.toString();
	}

	@Override
	public boolean equals(Object obj) {
		if(obj == this) {
			return true;
		}

		if(!(obj instanceof StackTraceElement)) {
			return false;
		}

		StackTraceElement a = this;
		StackTraceElement b = (StackTraceElement) obj;

		return
			Objects.equals(a.getClassLoaderName(), b.getClassLoaderName()) &&
			Objects.equals(a.getModuleName(), b.getModuleName()) &&
			Objects.equals(a.getModuleVersion(), b.getModuleVersion()) &&
			Objects.equals(a.getClassName(), b.getClassName()) &&
			Objects.equals(a.getMethodName(), b.getMethodName()) &&
			Objects.equals(a.getFileName(), b.getFileName()) &&
			a.getLineNumber() == b.getLineNumber();
	}

	public int hashCode() {
		int result = 0;
		result = 31 * result + Objects.hashCode(classLoaderName_);
		result = 31 * result + Objects.hashCode(moduleName_);
		result = 31 * result + Objects.hashCode(moduleVersion_);
		result = 31 * declaringClass_.hashCode() + methodName_.hashCode();
		result = 31 * result + Objects.hashCode(fileName_);
		result = 31 * result + lineNumber_;
		return result;
	}

}