// Get default algorithm preferences from localStorage
export const getDefaults = () => ({
  encrypt: localStorage.getItem('defaultEncrypt') || 'aes-256-gcm',
  hash: localStorage.getItem('defaultHash') || 'sha256',
  compression: localStorage.getItem('defaultCompression') || 'lzma',
  kdf: localStorage.getItem('defaultKdf') || 'argon2id',
});

// Operation statistics interface
export interface OperationStats {
  totalOperations: number;
  successfulOperations: number;
  failedOperations: number;
  totalFilesProcessed: number;
  totalBytesProcessed: number;
  lastOperation?: {
    type: string;
    timestamp: Date;
    success: boolean;
  };
}

// Get operation statistics from localStorage
export const getOperationStats = (): OperationStats => {
  const stored = localStorage.getItem('operationStats');
  if (stored) {
    const parsed = JSON.parse(stored);
    if (parsed.lastOperation) {
      parsed.lastOperation.timestamp = new Date(parsed.lastOperation.timestamp);
    }
    return parsed;
  }
  return {
    totalOperations: 0,
    successfulOperations: 0,
    failedOperations: 0,
    totalFilesProcessed: 0,
    totalBytesProcessed: 0,
  };
};

// Update operation statistics
export const updateOperationStats = (
  type: string,
  success: boolean,
  filesProcessed: number = 1,
  bytesProcessed: number = 0
) => {
  const stats = getOperationStats();
  stats.totalOperations++;
  if (success) {
    stats.successfulOperations++;
  } else {
    stats.failedOperations++;
  }
  stats.totalFilesProcessed += filesProcessed;
  stats.totalBytesProcessed += bytesProcessed;
  stats.lastOperation = {
    type,
    timestamp: new Date(),
    success,
  };
  localStorage.setItem('operationStats', JSON.stringify(stats));
};

// Format bytes to human readable
export const formatBytes = (bytes: number): string => {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return Math.round((bytes / Math.pow(k, i)) * 100) / 100 + ' ' + sizes[i];
};

// Recent files interface
export interface RecentFile {
  path: string;
  type: 'encrypt' | 'decrypt' | 'hash' | 'sign' | 'verify' | 'archive' | 'compress' | 'stego';
  timestamp: Date;
  success: boolean;
}

// Get recent files from localStorage (max 20)
export const getRecentFiles = (): RecentFile[] => {
  const stored = localStorage.getItem('recentFiles');
  if (stored) {
    const parsed = JSON.parse(stored);
    return parsed.map((file: any) => ({
      ...file,
      timestamp: new Date(file.timestamp),
    }));
  }
  return [];
};

// Add a file to recent files
export const addRecentFile = (path: string, type: RecentFile['type'], success: boolean) => {
  const recent = getRecentFiles();
  // Remove if already exists
  const filtered = recent.filter(f => f.path !== path);
  // Add to beginning
  filtered.unshift({
    path,
    type,
    timestamp: new Date(),
    success,
  });
  // Keep only last 20
  const limited = filtered.slice(0, 20);
  localStorage.setItem('recentFiles', JSON.stringify(limited));
};

// Clear recent files
export const clearRecentFiles = () => {
  localStorage.removeItem('recentFiles');
};