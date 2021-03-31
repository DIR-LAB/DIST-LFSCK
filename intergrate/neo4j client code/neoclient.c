
#include <neo4j-client.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    neo4j_client_init();



    /* use NEO4J_INSECURE when connecting to disable TLS */
    neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:sk@localhost:11003", NULL, NEO4J_INSECURE);
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        return EXIT_FAILURE;
    }

clock_t begin = clock();


neo4j_result_stream_t *results =
//root mds
neo4j_run(connection, "CREATE (:mds {Inode_num: 0,lma:10,linkEA:0,lovEA:0});", neo4j_null);

//child mds
neo4j_run(connection, "CREATE (:mds {Inode_num: 1,lma:20,linkEA:10,lovEA:11});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:mds) where a.linkEA=b.lma CREATE (b)-[r:parent_of]->(a);" , neo4j_null);
neo4j_run(connection, "CREATE (:mds {Inode_num: 2,lma:30,linkEA:10,lovEA:12});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:mds) where a.linkEA=b.lma CREATE (b)-[r:parent_of]->(a);" , neo4j_null);

//oss
neo4j_run(connection, "CREATE (:oss {Inode_num: 3,lma:11,FID:20});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lovEA=b.lma CREATE (a)-[:mds_parent_of]->(b);" , neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lma=b.FID CREATE (b)-[:oss_child_of]->(a);" , neo4j_null);
neo4j_run(connection, "CREATE (:oss {Inode_num: 4,lma:12,FID:30});", neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lovEA=b.lma CREATE (a)-[:mds_parent_of]->(b);" , neo4j_null);
neo4j_run(connection, "match (a:mds), (b:oss) where a.lma=b.FID CREATE (b)-[:oss_child_of]->(a);" , neo4j_null);


clock_t end = clock();
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
printf("%f seconds \n", time_spent);

		
	
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to run statement");
        return EXIT_FAILURE;
    }

   




    neo4j_close_results(results);
    neo4j_close(connection);
    neo4j_client_cleanup();
    return EXIT_SUCCESS;
}














