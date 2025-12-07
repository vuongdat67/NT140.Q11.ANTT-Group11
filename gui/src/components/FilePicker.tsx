import { useCallback } from 'react';
import { Upload } from 'lucide-react';
import { cn } from '../lib/utils';
import { open } from '@tauri-apps/plugin-dialog';

interface FilePickerProps {
  value?: string;
  onChange: (path: string) => void;
  accept?: string[];
  multiple?: boolean;
  directory?: boolean;
  className?: string;
  placeholder?: string;
}

export function FilePicker({
  value,
  onChange,
  accept,
  multiple = false,
  directory = false,
  className,
  placeholder = 'Click to select file...',
}: FilePickerProps) {
  const handleClick = useCallback(async () => {
    try {
      console.log('[FilePicker] Opening dialog with:', { multiple, directory, accept });
      
      // Use Tauri dialog API to get full file paths
      const selected = await open({
        multiple,
        directory,
        filters: accept ? [{ name: 'Files', extensions: accept }] : undefined,
      });

      console.log('[FilePicker] Selected:', selected);

      if (selected) {
        if (Array.isArray(selected)) {
          // Multiple files - return first one for now
          console.log('[FilePicker] Multiple files, using first:', selected[0]);
          onChange(selected[0]);
        } else {
          // Single file
          console.log('[FilePicker] Single file:', selected);
          onChange(selected);
        }
      } else {
        console.log('[FilePicker] User cancelled');
      }
    } catch (error) {
      console.error('[FilePicker] Failed to open file picker:', error);
    }
  }, [multiple, directory, accept, onChange]);

  return (
    <div
      onClick={handleClick}
      className={cn(
        'flex items-center gap-2 px-4 py-2 border border-border rounded-md cursor-pointer hover:bg-accent transition-colors',
        className
      )}
    >
      <Upload className="w-4 h-4 text-muted-foreground" />
      <span className={cn('text-sm', value ? 'text-foreground' : 'text-muted-foreground')}>
        {value || placeholder}
      </span>
    </div>
  );
}
