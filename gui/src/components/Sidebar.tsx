import { NavLink } from 'react-router-dom';
import {
  Lock,
  Unlock,
  Hash,
  Key,
  FileText,
  Archive,
  Image,
  Settings,
  Shield,
  Zap,
  Info,
} from 'lucide-react';
import { cn } from '../lib/utils';

const navItems = [
  { path: '/', icon: Shield, label: 'Dashboard' },
  { path: '/encrypt', icon: Lock, label: 'Encrypt' },
  { path: '/decrypt', icon: Unlock, label: 'Decrypt' },
  { path: '/hash', icon: Hash, label: 'Hash' },
  { path: '/keygen', icon: Key, label: 'Keygen' },
  { path: '/sign', icon: FileText, label: 'Sign' },
  { path: '/verify', icon: FileText, label: 'Verify' },
  { path: '/stego', icon: Image, label: 'Steganography' },
  { path: '/archive', icon: Archive, label: 'Archive' },
  { path: '/compress', icon: Archive, label: 'Compress' },
  { path: '/benchmark', icon: Zap, label: 'Benchmark' },
  { path: '/info', icon: Info, label: 'File Info' },
  { path: '/settings', icon: Settings, label: 'Settings' },
];

export function Sidebar() {
  return (
    <aside className="w-64 bg-base-200 border-r border-base-300 flex flex-col">
      <div className="p-6 border-b border-base-300">
        <div className="flex items-center gap-3">
          <Shield className="w-8 h-8 text-primary" />
          <div>
            <h1 className="text-xl font-bold">FileVault</h1>
            <p className="text-xs text-base-content/60">Secure File Manager</p>
          </div>
        </div>
      </div>

      <nav className="flex-1 p-4 space-y-1 overflow-y-auto">
        {navItems.map((item) => (
          <NavLink
            key={item.path}
            to={item.path}
            className={({ isActive }) =>
              cn(
                'flex items-center gap-3 px-3 py-2 rounded-md transition-colors',
                isActive
                  ? 'bg-primary text-primary-content'
                  : 'text-base-content/70 hover:bg-base-300 hover:text-base-content'
              )
            }
          >
            <item.icon className="w-5 h-5" />
            <span className="text-sm font-medium">{item.label}</span>
          </NavLink>
        ))}
      </nav>

      <div className="p-4 border-t border-base-300 text-xs text-base-content/60">
        <p>Version 1.0.0</p>
        <p className="mt-1">© 2025 FileVault</p>
      </div>
    </aside>
  );
}
