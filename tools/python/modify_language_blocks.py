#!/usr/bin/env python3
import os
import re
import glob

def get_language_from_filepath(filepath):
    """Extract language from filepath - either from filename suffix or folder name"""
    # Check filename suffix first
    if filepath.endswith('_de.inc'):
        return 'DE'
    elif filepath.endswith('_fr.inc'):
        return 'FR'
    elif filepath.endswith('_it.inc'):
        return 'IT'
    elif filepath.endswith('_es.inc'):
        return 'ES'
    
    # Check folder name
    if '/german/' in filepath:
        return 'DE'
    elif '/french/' in filepath:
        return 'FR'
    elif '/italian/' in filepath:
        return 'IT'
    elif '/spanish/' in filepath:
        return 'ES'
    
    return None

def process_file(filepath):
    """Process a single .inc file to add language suffixes to .string blocks"""
    language = get_language_from_filepath(filepath)
    if not language:
        print(f"Skipping {filepath} - no language suffix found")
        return False
    
    print(f"Processing {filepath} with language {language}")
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except UnicodeDecodeError:
        try:
            with open(filepath, 'r', encoding='latin-1') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
    
    # Split content into lines
    lines = content.split('\n')
    new_lines = []
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Check if this line is a block title (ends with ::)
        if line.strip().endswith('::'):
            # Look ahead to see if this block contains .string
            has_string = False
            j = i + 1
            
            # Look for .string in the next few lines (until next block title or end)
            while j < len(lines) and not lines[j].strip().endswith('::'):
                if '.string' in lines[j]:
                    has_string = True
                    break
                j += 1
            
            # If block contains .string and doesn't already have language suffix
            if has_string:
                block_name = line.strip()[:-2]  # Remove ::
                
                # Check if it already has a language suffix
                if not (block_name.endswith('_DE') or block_name.endswith('_FR') or 
                       block_name.endswith('_IT') or block_name.endswith('_ES')):
                    # Add language suffix
                    new_block_name = f"{block_name}_{language}::"
                    new_lines.append(new_block_name)
                    print(f"  Modified: {block_name} -> {block_name}_{language}")
                else:
                    # Keep as is (already has language suffix)
                    new_lines.append(line)
            else:
                # Keep as is (no .string found)
                new_lines.append(line)
        else:
            # Regular line, keep as is
            new_lines.append(line)
        
        i += 1
    
    # Write back the modified content
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(new_lines))
        print(f"  Successfully updated {filepath}")
        return True
    except Exception as e:
        print(f"Error writing {filepath}: {e}")
        return False

def find_language_files(base_dir):
    """Find all .inc files with language suffixes or in language folders"""
    patterns = [
        os.path.join(base_dir, '**', '*_de.inc'),
        os.path.join(base_dir, '**', '*_fr.inc'),
        os.path.join(base_dir, '**', '*_it.inc'),
        os.path.join(base_dir, '**', '*_es.inc'),
        os.path.join(base_dir, '**', 'german', '*.inc'),
        os.path.join(base_dir, '**', 'french', '*.inc'),
        os.path.join(base_dir, '**', 'italian', '*.inc'),
        os.path.join(base_dir, '**', 'spanish', '*.inc')
    ]
    
    files = []
    for pattern in patterns:
        files.extend(glob.glob(pattern, recursive=True))
    
    return files

def main():
    base_dir = '/Users/fpeiro/project2/pokefirered-europe'
    
    print("Finding language files...")
    files = find_language_files(base_dir)
    
    print(f"Found {len(files)} files to process")
    
    processed = 0
    successful = 0
    
    for filepath in files:
        processed += 1
        if process_file(filepath):
            successful += 1
    
    print(f"\nSummary:")
    print(f"Total files processed: {processed}")
    print(f"Successfully updated: {successful}")
    print(f"Failed: {processed - successful}")

if __name__ == "__main__":
    main()
