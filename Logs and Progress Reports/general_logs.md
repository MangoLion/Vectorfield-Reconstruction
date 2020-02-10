# 02/07/2020
- Re-designed development steps for finding segment neighboors in graph-representation framework. (See **progress report 02-07-2020.pptx**)
    - Major potential challenge: Since 3D rotation's orientation is ambiguious, how to "anchor" the rotation to get a consistent orientation in result?
        - Need to resolve 3D rotation's orientation issue. Proposals:
            - base the 3D rotation on the given 3D vector field orientation. **(Ask)**


# 02/10/2020
- Modified streamline reading code (reader.cpp) to separate the streamline reading from input file and the segment building process (into graph.cpp). Reason: for comprehensive segment resolution adjustment, allowing custom segment resolution. (For this to happen, streamlines must be read beforehand rather than build the segment during streamline reading)
- Implementing adaptive streamline read: Only keep streamline nodes at desired features (change in sign of vector angle rotation derivative)
    - Problem: It's easy to detect direction change by detecting a change in sign of vector angle rotation derivative in 2D
    But will it work in 3D? Rephrase: can a change in sign of vector angle rotation derivative in 3D be a good indicator of an interesting feature of that 3D streamline?
        - Proposal: Since 3D angle is calculated from vector's vx vy vz, just need to keep track when the sign of dvx, dvy, or dvz changes and mark as critical point
        Apply this to 2D too, rather than calculating vector angle from Atan()
## Dr.Chen Meeting @ 1pm
- Look into KDG and how it is similar to current planned graph framework
- Reimplement the graph framework, rather than segments, create direct edges (for each SL segment), and neighboring edges (for neighboring segments). Then populate the graph with the edges to form a connected graph.
    - Implement graph resolution based on the direct edges, **only merge edges that share the same neighboring edge** .
        - Ex: e1 -> g1, e2 -> g1, e3 -> g2. Only merge e1 and e2
- For 3D neighbor segment search
    - parameterize the number of cones search around each segment.
        Implementation: create a single function that construct the "search lines" (5 search lines for each side, 4 lines for front and back),  parameterize angle scaling. Then have an outer wrapper function that determines the angle scaling and how many times to call the first function

- For adaptive streamline reading (look for critical features such as critical points), look into **sign curvature**