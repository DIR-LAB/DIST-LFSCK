
#include <neo4j-client.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    neo4j_client_init();

//bolt://localhost:11005
//neo4j://user:pass@localhost:7687
//neo4j://neo4j:neo4j@localhost:7687

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




/*
neo4j_run(connection,"CREATE (a:MEH)-[r:parent_of]->(b:MEH {Inode_num: 2})" , neo4j_null);
*/
//neo4j_run(connection, "match (c:MDS {Inode_num: 1,lma:10,linkEA:0,lovEA:0}) delete c;" , neo4j_null);
//neo4j_run(connection, "delete c;" , neo4j_null);

//neo4j_run(connection, "match (a {inode:num : 1})", neo4j_null);
//neo4j_run(connection, "return a.lma;" , neo4j_null);
//neo4j_run(connection, "MATCH (a:MDS),(b:MDS) WHERE a.linkEA=b.lma", neo4j_null);
//neo4j_run(connection, "CREATE (a)-[r:parent_of]->(b)", neo4j_null);


 
    //neo4j_run(connection, "RETURN 'hello world'", neo4j_null);
		
	
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to run statement");
        return EXIT_FAILURE;
    }

   


/*
    neo4j_result_t *result = neo4j_fetch_next(results);
    if (result == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        return EXIT_FAILURE;
    }

    neo4j_value_t value = neo4j_result_field(result, 0);
    char buf[128];
    printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));

*/

    neo4j_close_results(results);
    neo4j_close(connection);
    neo4j_client_cleanup();
    return EXIT_SUCCESS;
}









/*create (:MDS {inode_num: 1, lma: 2})
match (a:MDS)
where a.inode-num = '1'
return a;


match (a {inode_num: 1})
return a;
*/

/* works:
neo4j_run(connection, "match (c:MDS {Inode_num: 1,lma:10,linkEA:0,lovEA:0}) delete c;" , neo4j_null);




match (a:MEh), (b:MEh)
where a.linkEA=b.lma
CREATE (a)-[r:parent_of]->(b)

*/













