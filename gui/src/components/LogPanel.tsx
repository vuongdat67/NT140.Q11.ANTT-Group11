import React from 'react';
import type { LogEntry } from '../types';
import { AlertCircle, CheckCircle2, Info, AlertTriangle } from 'lucide-react';
import { cn } from '../lib/utils';

interface LogPanelProps {
  logs: LogEntry[];
  className?: string;
  maxHeight?: string;
}

export function LogPanel({ logs, className, maxHeight = '300px' }: LogPanelProps) {
  const logRef = React.useRef<HTMLDivElement>(null);

  React.useEffect(() => {
    if (logRef.current) {
      logRef.current.scrollTop = logRef.current.scrollHeight;
    }
  }, [logs]);

  const getIcon = (level: LogEntry['level']) => {
    switch (level) {
      case 'info':
        return <Info className="w-4 h-4 text-blue-500" />;
      case 'success':
        return <CheckCircle2 className="w-4 h-4 text-green-500" />;
      case 'warning':
        return <AlertTriangle className="w-4 h-4 text-yellow-500" />;
      case 'error':
        return <AlertCircle className="w-4 h-4 text-red-500" />;
    }
  };

  const getLevelColor = (level: LogEntry['level']) => {
    switch (level) {
      case 'info':
        return 'text-info';
      case 'success':
        return 'text-success';
      case 'warning':
        return 'text-warning';
      case 'error':
        return 'text-error';
    }
  };

  return (
    <div className={cn('bg-base-200 border border-base-300 rounded-md', className)}>
      <div className="px-4 py-2 border-b border-base-300 bg-base-300">
        <h3 className="text-sm font-semibold text-base-content">Logs</h3>
      </div>
      <div
        ref={logRef}
        className="overflow-y-auto p-4 space-y-2 font-mono text-xs bg-base-100"
        style={{ maxHeight }}
      >
        {logs.length === 0 ? (
          <p className="text-base-content/60">No logs yet...</p>
        ) : (
          logs.map((log, index) => (
            <div key={index} className="flex items-start gap-2 py-1">
              {getIcon(log.level)}
              <span className="text-base-content/70 shrink-0">
                {log.timestamp.toLocaleTimeString()}
              </span>
              <span className={cn(getLevelColor(log.level), 'whitespace-pre-wrap break-words flex-1')}>
                {log.message}
              </span>
            </div>
          ))
        )}
      </div>
    </div>
  );
}
