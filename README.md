# Password-security-manager
This C project is a safe local password manager. You log in with a master password, and it loads your saved data from a file. You can add, search, sort, update, or delete passwords. It never saves your real password—only a secure hash! Plus, a stack lets you undo mistakes easily.



Menu Options & DSA Used
1. Add New Password: Saves your site name, username, and password hash using a Hash Table, Hashing, Trie, and Stack.   
2. Search Password: Finds accounts quickly using an exact Hash Table match or Trie suggestions.
3. View All (Sorted): Displays all saved passwords in alphabetical order using Merge Sort.
4. Update Password: Modifies existing passwords safely.
5. Delete Password: Removes unwanted entries from the system.
6. Undo Last Change: Reverts your last action instantly using a Stack.
7. Exit: Saves all your data to a text file using File Handling.

Note: Site names and usernames should be one word (no spaces).
Default Master Password: admin123 

gcc password_manager-2.c
./a.out (for linux)
./a.exe (for window)
