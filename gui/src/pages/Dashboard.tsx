import { Activity, FileText, CheckCircle2, XCircle, FileIcon, Trash2, ExternalLink, FolderOpen } from 'lucide-react';
import { Card } from '../components/Card';
import { useState, useEffect } from 'react';
import { getOperationStats, formatBytes, getRecentFiles, clearRecentFiles, type RecentFile } from '../lib/preferences';
import { Button } from '../components/Button';
import { open } from '@tauri-apps/plugin-shell';

export function Dashboard() {
  const [stats, setStats] = useState(getOperationStats());
  const [recentFiles, setRecentFiles] = useState<RecentFile[]>(getRecentFiles());

  // Try raw path first (Windows friendly), fallback to file:// URI
  const openPath = async (target: string) => {
    try {
      await open(target);
      return;
    } catch (err) {
      console.error('shell.open raw path failed, retrying file://', err);
    }

    const fileUrl = target.startsWith('file://')
      ? target
      : 'file://' + target.replace(/\\/g, '/');
    await open(fileUrl);
  };

  useEffect(() => {
    // Refresh stats and recent files every 1 second for better responsiveness
    const interval = setInterval(() => {
      setStats(getOperationStats());
      setRecentFiles(getRecentFiles());
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  const successRate = stats.totalOperations > 0 
    ? Math.round((stats.successfulOperations / stats.totalOperations) * 100)
    : 0;

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Dashboard</h1>
        <p className="text-base-content/70 mt-2">
          Welcome to FileVault - Your secure file encryption solution
        </p>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <div className="stat bg-base-200 rounded-lg shadow">
          <div className="stat-figure text-primary">
            <Activity className="w-8 h-8" />
          </div>
          <div className="stat-title">Total Operations</div>
          <div className="stat-value text-primary">{stats.totalOperations}</div>
          <div className="stat-desc">All time operations</div>
        </div>

        <div className="stat bg-base-200 rounded-lg shadow">
          <div className="stat-figure text-success">
            <CheckCircle2 className="w-8 h-8" />
          </div>
          <div className="stat-title">Successful</div>
          <div className="stat-value text-success">{stats.successfulOperations}</div>
          <div className="stat-desc">{successRate}% success rate</div>
        </div>

        <div className="stat bg-base-200 rounded-lg shadow">
          <div className="stat-figure text-error">
            <XCircle className="w-8 h-8" />
          </div>
          <div className="stat-title">Failed</div>
          <div className="stat-value text-error">{stats.failedOperations}</div>
          <div className="stat-desc">Errors encountered</div>
        </div>

        <div className="stat bg-base-200 rounded-lg shadow">
          <div className="stat-figure text-info">
            <FileText className="w-8 h-8" />
          </div>
          <div className="stat-title">Files Processed</div>
          <div className="stat-value text-info">{stats.totalFilesProcessed}</div>
          <div className="stat-desc">{formatBytes(stats.totalBytesProcessed)} total</div>
        </div>
      </div>

      {stats.lastOperation && (
        <div className="alert alert-info shadow-lg">
          <div>
            <span className="font-semibold">Last Operation:</span>
            <span className="ml-2">{stats.lastOperation.type}</span>
            <span className="ml-2 text-sm opacity-70">
              {stats.lastOperation.timestamp.toLocaleString()}
            </span>
            <span className={`ml-2 badge ${stats.lastOperation.success ? 'badge-success' : 'badge-error'}`}>
              {stats.lastOperation.success ? 'Success' : 'Failed'}
            </span>
          </div>
        </div>
      )}

      <Card title="Quick Start" description="Get started with FileVault">
        <div className="space-y-4">
          <div className="flex items-start gap-3">
            <div className="w-8 h-8 rounded-full bg-primary text-primary-foreground flex items-center justify-center font-bold">
              1
            </div>
            <div>
              <h3 className="font-semibold">Select a file to encrypt</h3>
              <p className="text-sm text-base-content/70">
                Navigate to Encrypt page and choose your file
              </p>
            </div>
          </div>

          <div className="flex items-start gap-3">
            <div className="w-8 h-8 rounded-full bg-primary text-primary-content flex items-center justify-center font-bold">
              2
            </div>
            <div>
              <h3 className="font-semibold">Choose encryption algorithm</h3>
              <p className="text-sm text-base-content/70">
                Select from AES-256-GCM, ChaCha20-Poly1305, and more
              </p>
            </div>
          </div>

          <div className="flex items-start gap-3">
            <div className="w-8 h-8 rounded-full bg-primary text-primary-content flex items-center justify-center font-bold">
              3
            </div>
            <div>
              <h3 className="font-semibold">Set password and encrypt</h3>
              <p className="text-sm text-base-content/70">
                Enter a strong password and click Encrypt
              </p>
            </div>
          </div>
        </div>
      </Card>

      <Card title="Recent Files" description="Your recently processed files">
        <div className="space-y-2">
          {recentFiles.length === 0 ? (
            <p className="text-base-content/60 text-sm">No recent files</p>
          ) : (
            <>
              <div className="flex justify-end mb-2">
                <Button
                  variant="ghost"
                  size="sm"
                  onClick={() => {
                    clearRecentFiles();
                    setRecentFiles([]);
                  }}
                >
                  <Trash2 className="w-4 h-4 mr-1" />
                  Clear All
                </Button>
              </div>
              <div className="space-y-2 max-h-96 overflow-y-auto">
                {recentFiles.map((file, index) => (
                  <div
                    key={index}
                    className="flex items-center justify-between p-3 bg-base-300 rounded-lg hover:bg-base-content/5 transition-colors"
                  >
                    <div className="flex items-center gap-3 flex-1 min-w-0">
                      <FileIcon className="w-5 h-5 text-primary shrink-0" />
                      <div className="flex-1 min-w-0">
                        <p className="text-sm font-medium truncate">{file.path.split(/[/\\]/).pop()}</p>
                        <div className="flex items-center gap-2 mt-1">
                          <span className="text-xs text-base-content/60 capitalize">{file.type}</span>
                          <span className="text-xs text-base-content/40">•</span>
                          <span className="text-xs text-base-content/60">
                            {file.timestamp.toLocaleString()}
                          </span>
                        </div>
                      </div>
                    </div>
                    <div className="flex items-center gap-2 shrink-0">
                      <Button
                        variant="ghost"
                        size="icon"
                        onClick={async () => {
                          try {
                            if (!file.path) {
                              console.error('File path is empty');
                              return;
                            }
                            await openPath(file.path);
                          } catch (error) {
                            console.error('Failed to open file:', error);
                            alert(`Failed to open file: ${file.path}\n${error instanceof Error ? error.message : String(error)}`);
                          }
                        }}
                        title="Open file"
                      >
                        <ExternalLink className="w-4 h-4" />
                      </Button>
                      <Button
                        variant="ghost"
                        size="icon"
                        onClick={async () => {
                          try {
                            const dir = file.path.replace(/[/\\][^/\\]+$/, '');
                            if (dir) {
                              await openPath(dir);
                            }
                          } catch (error) {
                            console.error('Failed to open folder:', error);
                            const dir = file.path.replace(/[/\\][^/\\]+$/, '');
                            alert(`Failed to open folder: ${dir}\n${error instanceof Error ? error.message : String(error)}`);
                          }
                        }}
                        title="Show in folder"
                      >
                        <FolderOpen className="w-4 h-4" />
                      </Button>
                      {file.success ? (
                        <CheckCircle2 className="w-5 h-5 text-success" />
                      ) : (
                        <XCircle className="w-5 h-5 text-error" />
                      )}
                    </div>
                  </div>
                ))}
              </div>
            </>
          )}
        </div>
      </Card>
    </div>
  );
}
