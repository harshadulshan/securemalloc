using Microsoft.Data.Sqlite;

class Dashboard
{
    // Change this path if your db is elsewhere
    const string DB_PATH = "D:\\traning project\\securemalloc\\securemalloc.db";

    static void Main(string[] args)
    {
        Console.Title = "SecureMalloc Dashboard";

        const int REFRESH_SECONDS = 5;

        while (true)
        {
            Console.Clear();
            PrintDashboard();

            Console.WriteLine($"Auto-refreshing every {REFRESH_SECONDS}s");
            Console.WriteLine("Press R to refresh | Press E to export CSV | Press Q to quit");

            var stopwatch = System.Diagnostics.Stopwatch.StartNew();
            while (stopwatch.Elapsed.TotalSeconds < REFRESH_SECONDS)
            {
                if (Console.KeyAvailable)
                {
                    var key = Console.ReadKey(true);
                    if (key.Key == ConsoleKey.Q) return;
                    if (key.Key == ConsoleKey.R) break;
                    if (key.Key == ConsoleKey.E)
                    {
                        ExportToCsv();
                        Console.WriteLine("  Press any key to continue...");
                        Console.ReadKey(true);
                        break;
                    }
                }
                Thread.Sleep(100);
            }
        }
    }

    static void PrintDashboard()
    {
        Console.WriteLine("==========================================");
        Console.WriteLine("        SECUREMALLOC LIVE DASHBOARD       ");
        Console.WriteLine("==========================================");
        Console.WriteLine();

        if (!File.Exists(DB_PATH))
        {
            Console.WriteLine("ERROR: Database not found at:");
            Console.WriteLine(DB_PATH);
            Console.WriteLine("Run test_db.exe first to generate it.");
            return;
        }

        using var connection = new SqliteConnection($"Data Source={DB_PATH}");
        connection.Open();

        // --- Summary ---
        Console.WriteLine("  SUMMARY");
        Console.WriteLine("------------------------------------------");

        var summaryCmd = connection.CreateCommand();
        summaryCmd.CommandText =
            "SELECT status, COUNT(*) as count, COALESCE(SUM(size),0) as total " +
            "FROM allocations GROUP BY status;";

        using (var reader = summaryCmd.ExecuteReader())
        {
            while (reader.Read())
            {
                string status = reader.GetString(0);
                long   count  = reader.GetInt64(1);
                long   total  = reader.GetInt64(2);

                // Color coding
                if (status == "LIVE")
                    Console.ForegroundColor = ConsoleColor.Red;
                else
                    Console.ForegroundColor = ConsoleColor.Green;

                Console.WriteLine($"  {status,-8} | {count,4} blocks | {total,8} bytes");
                Console.ResetColor();
            }
        }

        Console.WriteLine();

        // --- Live blocks (leaks) ---
        var liveCmd = connection.CreateCommand();
        liveCmd.CommandText =
            "SELECT address, size, file, line " +
            "FROM allocations WHERE status='LIVE' ORDER BY id DESC;";

        using (var reader = liveCmd.ExecuteReader())
        {
            bool hasLeaks  = false;
            int  leakCount = 0;

            // Read all leaks into a list first
            var leaks = new List<(string address, long size, string file, long line)>();
            while (reader.Read())
            {
                leaks.Add((
                    reader.GetString(0),
                    reader.GetInt64(1),
                    reader.GetString(2),
                    reader.GetInt64(3)
                ));
            }

            if (leaks.Count > 0)
            {
                hasLeaks  = true;
                leakCount = leaks.Count;

                // Determine overall severity
                bool isCritical = leaks.Count > 1 ||
                                  leaks.Any(l => l.size >= 100);

                if (isCritical)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine("  [CRITICAL] SEVERE LEAKS DETECTED!");
                }
                else
                {
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.WriteLine("  [WARNING] LEAKS DETECTED!");
                }

                Console.WriteLine("------------------------------------------");
                Console.ResetColor();

                foreach (var leak in leaks)
                {
                    // Per leak severity
                    string severity = leak.size >= 100 ? "[CRITICAL]" : "[WARNING] ";

                    if (leak.size >= 100)
                        Console.ForegroundColor = ConsoleColor.Red;
                    else
                        Console.ForegroundColor = ConsoleColor.Yellow;

                    Console.WriteLine(
                        $"  {severity} {leak.address} | {leak.size,6} bytes" +
                        $" | {leak.file} | line {leak.line}");
                    Console.ResetColor();
                }
            }

            if (!hasLeaks)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("  [SAFE] No leaks — All memory freed correctly!");
                Console.ResetColor();
            }

            // Severity summary line
            Console.WriteLine();
            if (leakCount == 0)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("  Overall Status : SAFE");
            }
            else if (leakCount == 1 && leaks.All(l => l.size < 100))
            {
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("  Overall Status : WARNING");
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("  Overall Status : CRITICAL");
            }
            Console.ResetColor();
        }

        // --- Total stats ---
        Console.WriteLine();
        Console.WriteLine("------------------------------------------");

        var totalCmd = connection.CreateCommand();
        totalCmd.CommandText =
            "SELECT COUNT(*), COALESCE(SUM(size),0) FROM allocations;";

        using (var reader = totalCmd.ExecuteReader())
        {
            if (reader.Read())
            {
                Console.WriteLine($"  Total records : {reader.GetInt64(0)}");
                Console.WriteLine($"  Total bytes   : {reader.GetInt64(1)}");
            }
        }

        PrintGraph();
        Console.WriteLine("==========================================");
        Console.WriteLine($"  Last updated  : {DateTime.Now}");
        Console.WriteLine("==========================================");
        Console.WriteLine();
    }
static void ExportToCsv()
    {
        string exportPath = "D:\\traning project\\securemalloc\\leak_report.csv";

        using var connection = new SqliteConnection($"Data Source={DB_PATH}");
        connection.Open();

        var cmd = connection.CreateCommand();
        cmd.CommandText =
            "SELECT address, size, file, line, status, time " +
            "FROM allocations ORDER BY id ASC;";

        using var writer = new StreamWriter(exportPath);

        // Header row
        writer.WriteLine("address,size_bytes,file,line,status,time");

        using var reader = cmd.ExecuteReader();
        int count = 0;
        while (reader.Read())
        {
            string address = reader.GetString(0);
            long   size    = reader.GetInt64(1);
            string file    = reader.GetString(2);
            long   line    = reader.GetInt64(3);
            string status  = reader.GetString(4);
            string time    = reader.GetString(5);

            writer.WriteLine($"{address},{size},{file},{line},{status},{time}");
            count++;
        }

        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine($"  CSV exported: {count} records");
        Console.WriteLine($"  Location: {exportPath}");
        Console.ResetColor();
    }

static void PrintGraph()
    {
        Console.WriteLine();
        Console.WriteLine("  ALLOCATION HISTORY (by minute)");
        Console.WriteLine("------------------------------------------");

        using var connection = new SqliteConnection($"Data Source={DB_PATH}");
        connection.Open();

        var cmd = connection.CreateCommand();
        cmd.CommandText =
            "SELECT strftime('%H:%M', time) as minute, " +
            "COUNT(*) as count, " +
            "COALESCE(SUM(size), 0) as total " +
            "FROM allocations " +
            "GROUP BY minute " +
            "ORDER BY minute ASC " +
            "LIMIT 10;";

        using var reader = cmd.ExecuteReader();

        // Find max total for scaling
        var rows = new List<(string minute, long count, long total)>();
        while (reader.Read())
        {
            rows.Add((
                reader.GetString(0),
                reader.GetInt64(1),
                reader.GetInt64(2)
            ));
        }

        if (rows.Count == 0)
        {
            Console.WriteLine("  No data yet.");
            return;
        }

        long maxTotal = rows.Max(r => r.total);

        foreach (var row in rows)
        {
            // Scale bar to max 30 chars
            int barLength = maxTotal > 0
                ? (int)(row.total * 30 / maxTotal)
                : 0;

            string bar = new string('█', barLength);

            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write($"  {row.minute}  ");
            Console.ForegroundColor = ConsoleColor.Blue;
            Console.Write($"{bar,-30}");
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine($"  {row.total,6} bytes  ({row.count} allocs)");
            Console.ResetColor();
        }

        Console.WriteLine("------------------------------------------");
    }


}